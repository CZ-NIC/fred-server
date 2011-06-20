/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <memory>
#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/checked_delete.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "common_impl.h"
#include "invoice.h"
#include "log/logger.h"
#include "log/context.h"
#include "types/convert_sql_db_types.h"
#include "types/sqlize.h"

#include "documents.h"

#include "log/logger.h"

using namespace boost::gregorian;
using namespace boost::posix_time;


namespace Fred {
namespace Invoicing {

std::string
Type2Str(Type _type)
{
    switch (_type) {
        case IT_DEPOSIT:    return "Deposit";
        case IT_ACCOUNT:    return "Account";
        default:            return "TYPE UNKNOWN";
    }
}

std::string
PaymentActionType2Str(PaymentActionType type)
{
    switch (type) {
        case PAT_CREATE_DOMAIN: return "Create domain";
        case PAT_RENEW_DOMAIN:  return "Renew domain";
        default:                return "TYPE UNKNOWN";
    }
}

cent_amount get_price(const std::string &str)
{
  std::string t_str = boost::algorithm::trim_copy(str);//remove whitespaces
  std::size_t delimiter = t_str.find_first_of(".,");//find delimiter
  if (delimiter != std::string::npos)//if found
  {
        t_str.erase(delimiter,1);//erase delimiter
        if(t_str.length() > (delimiter + 2))//if there are more than two chars after delimiter
            t_str.erase(delimiter+2);//remove rest of the string
        else if (t_str.length() == (delimiter + 1))//if there is only one char after delimiter
            t_str+="0";//append second char after delimiter
        else if (t_str.length() == delimiter)//if there is no char after delimiter
          t_str+="00";//append first and second char after delimiter
  }
  else
  {
      t_str += "00";
  }

  unsigned long price = boost::lexical_cast<unsigned long>(t_str);//try convert
  LOGGER(PACKAGE).debug( boost::format("get_price from string[%1%] -> %2% hal") % str % price );
  return price;
}

// hold vat rates for time periods
class VAT {
public:
  VAT(unsigned _vatRate, unsigned _koef, date _validity) :
    vatRate(_vatRate), koef(_koef), validity(_validity) {
  }
  bool operator==(unsigned rate) const {
    return vatRate == rate;
  }
  unsigned vatRate; ///< percent rate
  unsigned koef; ///< koeficient for VAT counting (in 1/10000)
  date validity; ///< valid to this date
};
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  ManagerImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/// implementation forManagerAbstr interface
class ManagerImpl : public Manager {
    
  Document::Manager *docman;
  Mailer::Manager *mailman;
  std::vector<VAT> vatList;  
  
  void initVATList() ;
  
public:

  ManagerImpl() :docman(NULL), mailman(NULL) {
    initVATList();
  }

  
  ManagerImpl(Document::Manager *_doc_manager, Mailer::Manager *_mail_manager) :
    docman(_doc_manager), mailman(_mail_manager) {
    initVATList();
  }
  /// count vat from price
  /** price can be base or base+vat according to base flag */
  Money countVAT(Money price, unsigned vatRate, bool base);
  const VAT *getVAT(unsigned rate);
  /// find unarchived invoices. archive then and send them by email
  InvoiceIdVect archiveInvoices(bool send);
  /// create empty list of invoices      
  virtual List* createList();
  /// return credit for registrar by zone
  virtual std::string
      getCreditByZone(const std::string& registrarHandle, TID zone);
  virtual bool insertInvoicePrefix(unsigned long long zoneId,
          int type, int year, unsigned long long prefix);
  virtual bool insertInvoicePrefix(const std::string &zoneName,
          int type, int year, unsigned long long prefix);



  // TODO format %2% to 2 decimal places
  std::string query_param_price(cent_amount price)
  {
      return (boost::format("%1%.%2$02u") % (price/100) % (price >= 0  ? price%100 : (-1*price)%100) ).str();
  }

  void updateObjectPrice(Database::Connection &conn, Database::ID rec_id, cent_amount price, Database::ID invoiceID)
  {
      conn.exec_params("INSERT INTO invoice_object_registry_price_map (id, invoiceID, price) "
          "VALUES ($1::integer, $2::integer, $3::numeric(10,2)) ",
          Database::query_param_list( rec_id )
                                     (invoiceID)
                                     (query_param_price(price))
                                     );
  }

  // create new record in invoice_object_registry and return the ID
  Database::ID createInvoiceObjectRegistry(Database::Connection &conn, Database::ID objectID, Database::ID registrar, Database::ID zone, int period, const Database::Date &exDate, bool renew)
  {
      Database::Result res_id = conn.exec("SELECT nextval('invoice_object_registry_id_seq'::regclass) ");
      Database::ID id = res_id[0][0];

      conn.exec_params("INSERT INTO invoice_object_registry(id, objectid, registrarid, operation, zone, period, ExDate) VALUES ("
            "$1::integer, $2::integer, $3::integer, $4::integer, $5::integer, $6::integer, $7::date)",
            Database::query_param_list(  id  )
                                       (objectID)
                                       (registrar)
                                       (static_cast<int>(renew ? INVOICING_DomainRenew : INVOICING_DomainCreate))
                                       (zone)
                                       (period)
                                       (exDate));

      return id;
  }

  void invoiceLowerCredit(Database::Connection &conn, cent_amount price, Database::ID invoiceID) {

      if(price == 0) {
          LOGGER(PACKAGE).debug ( boost::format("Zero price for invoiceID %1%, credit unchanged.") % invoiceID);
          return;
      }
      conn.exec_params("UPDATE invoice SET credit = credit - $1::numeric(10,2) WHERE id=$2::integer",
              Database::query_param_list(  query_param_price(price) )
                                          (invoiceID));

  }

  // WARNING: this is called from epp_impl.cc and it's sharing connection with dbsql's DB
  // so it must *NOT* create Database::Transactions
  virtual bool domainBilling(
              const Database::ID &zone,
              const Database::ID &registrar,
              const Database::ID &objectId,
              const Database::Date &exDate,
              const int &units_count,
              bool renew) {
      try {
      // TODO we rely that connection is saved in thread specific data
      Database::Connection conn = Database::Manager::acquire();

      // find out whether the registrar in question is system registrar
      bool system = false;
      Database::Result rsys =
        conn.exec_params("SELECT system FROM registrar WHERE id=$1::integer",
            Database::query_param_list(registrar));
      if(rsys.size() != 1 || rsys[0][0].isnull()) {
          throw std::runtime_error((boost::format("Registrar ID %1% not found ") % registrar).str());
      } else {
          system = rsys[0][0];
          if(system) {
              LOGGER(PACKAGE).info ( (boost::format("Registrar ID %1% has system flag set, not billing") % registrar).str());
              // no billing for system registrar
              return true;
          }
      }

     // find operation price in pennies/cents:
     Database::Result res_price = conn.exec_params("SELECT price , period FROM price_list WHERE valid_from < 'now()'  "
              "and ( valid_to is NULL or valid_to > 'now()' ) "
              "and operation=$1::integer and zone=$2::integer "
              "order by valid_from desc limit 1",
         Database::query_param_list( static_cast<int>(renew ? INVOICING_DomainRenew : INVOICING_DomainCreate) )
                                     (zone ));

      if(res_price.size() != 1 || res_price[0][0].isnull()) {
          LOGGER(PACKAGE).info ( (boost::format("Operation %1% for zoneId %2% not found in price list. No billing.") % static_cast<int>(renew ? INVOICING_DomainRenew : INVOICING_DomainCreate)  % zone).str());
          // price not set - no billing
          return true;
      }

      cent_amount price = get_price((std::string)res_price[0][0]);
      if(units_count > 0) {
          if(res_price[0][1].isnull()) {
              throw std::runtime_error("Couldn't find price for this operation");
          }
          int base_period = res_price[0][1];
          if(base_period != 0) price *= (units_count / base_period);
      }

      // billing itself
      // lock invoices suitable for charging
      Database::Result res_inv = conn.exec_params("SELECT id, credit FROM invoice "
              "WHERE registrarid=$1::integer and zone=$2::integer and credit > 0 "
              "order by id limit 2 "
              "FOR UPDATE",
              Database::query_param_list(registrar)
                                        (zone));

      if(res_inv.size() == 0) {
          throw std::runtime_error((boost::format("No usable invoices found for registrar ID %1%") % registrar).str());
      }

      // do we have only one invoice which can be used?
      bool single_invoice = true;
      if(res_inv.size() != 1) {
          single_invoice = false;
      }

      Database::ID inv_id1 = res_inv[0][0];
      cent_amount credit1 = get_price((std::string)res_inv[0][1]);
      if(price <= credit1) {
          // count if off the first invoice

          LOGGER(PACKAGE).debug ( boost::format(
              "domainBilling: RegistrarId %1%, zoneId %2%, objectId %3%, price %4% exDate %5%, single invoice %6% ")
              % registrar % zone % objectId % query_param_price(price) % exDate
              % inv_id1);

          Database::ID invoice_obj_reg_id = createInvoiceObjectRegistry(conn, objectId, registrar, zone, units_count, exDate, renew);

          updateObjectPrice(conn, invoice_obj_reg_id, price, inv_id1);
          invoiceLowerCredit(conn, price, inv_id1);

      } else {
          // if single_invoice is set, res_inv[1][*] is not valid
          if(single_invoice) {
              LOGGER(PACKAGE).info((boost::format("Credit not sufficient for registrar ID %1%, invoice: %2%")
                  % registrar % inv_id1).str());
              return false;
          }
          // there is the second invoice

          Database::ID inv_id2 = res_inv[1][0];
          cent_amount credit2 = get_price((std::string)res_inv[1][1]);

          if(credit1 + credit2 < price) {
              LOGGER(PACKAGE).info((boost::format("Credit not sufficient for registrar ID %1% operation %2% on object %3%, invoices: %4%, %5%")
                   % registrar
                   % static_cast<int>(renew ? INVOICING_DomainRenew : INVOICING_DomainCreate)
                   % objectId % inv_id1 % inv_id2).str());
              return false;
          }

          LOGGER(PACKAGE).debug ( boost::format(
              "domainBilling: RegistrarId %1%, zoneId %2%, objectId %3%, price %4% exDate %5%, two invoices %6%, %7%")
              % registrar % zone % objectId % query_param_price(price) % exDate
              % inv_id1 % inv_id2);

          cent_amount price_remainder = price - credit1;

          Database::ID invoice_obj_reg_id = createInvoiceObjectRegistry(conn, objectId, registrar, zone, units_count, exDate, renew);

          updateObjectPrice(conn, invoice_obj_reg_id, credit1, inv_id1);
          updateObjectPrice(conn, invoice_obj_reg_id, price_remainder, inv_id2);

          // first invoice goes to zero balance
          invoiceLowerCredit(conn, credit1, inv_id1);
          // remaining price of the operation
          invoiceLowerCredit(conn, price_remainder, inv_id2);

      }

      } catch( std::exception &ex) {
          LOGGER(PACKAGE).error ( boost::format("Billing failed: %1% ") % ex.what());
          return false;
      }
      catch(...) {
                LOGGER(PACKAGE).error("Billing failed.");
                return false;
            }

      return true;

  }

  virtual bool chargeDomainCreate(
          const Database::ID &zone,
          const Database::ID &registrar,
          const Database::ID &objectId,
          const Database::Date &exDate,
          const int &units_count)
  {
      TRACE("[CALL] Fred::Invoicing::Manager::chargeDomainCreate()");
      // new implementation
      return domainBilling(zone, registrar, objectId, exDate, units_count, false);
  }

  virtual bool chargeDomainRenew(
          const Database::ID &zone,
          const Database::ID &registrar,
          const Database::ID &objectId,
          const Database::Date &exDate,
          const int &units_count)
  {
      TRACE("[CALL] Fred::Invoicing::Manager::chargeDomainRenew()");
      // new implementation
            return domainBilling(zone, registrar, objectId, exDate, units_count, true);
  }


  //  count VAT from price without tax with help of coefficient
  // count VAT  ( local CZ ) function for banking
  cent_amount count_dph( //returning vat in cents
          cent_amount _price //*100 in cents
    , cent_amount vat_reverse //vat coeff *10000
    )
    {
      unsigned long long price = _price;

      cent_amount vat =  (((price * vat_reverse) % 10000) < 5000)
                              ? price * vat_reverse / 10000
                              : price * vat_reverse / 10000 + 1;

         LOGGER(PACKAGE).debug (
             boost::format("count_dph price %1% vat_reverse %2% vat %3%")
             % price % vat_reverse % vat);

         return vat;
  }


    /**
     *returns 0 in case of failure, id of invoice otherwise
     * type is int just because MakeNewInoviceAdvance returns it.
     */

// TODO what exceptions to throw and whether to log errors
// SPEC current time taken from now() in DB (which should be UTC)
unsigned long long  createDepositInvoice(Database::Date date, int zoneId, int registrarId, cent_amount price)
{
    Database::Connection conn = Database::Manager::acquire();

    Database::Result rvat = conn.exec_params("SELECT vat FROM registrar WHERE id=$1::integer",
            Database::query_param_list(registrarId));

    if(rvat.size() != 1 || rvat[0][0].isnull() ) {
        throw std::runtime_error(
            (boost::format(" Couldn't determine whether the registrar with ID %1% pays VAT.")
                % registrarId).str());
    }
    bool pay_vat = rvat[0][0];

    //// handle VAT

    // VAT  percentage
    int vat_percent = 0;
    // ratio for reverse VAT calculation
    // price_without_vat = price_with_vat - price_with_vat*vat_reverse
    // it should have 4 decimal places in the DB
    long vat_reverse = 0;

    cent_amount vat_amount = 0;
    cent_amount total;

    if (pay_vat) {

        Database::Result vat_details = conn.exec_params(
                "select vat, koef*10000::numeric from price_vat where valid_to > $1::date or valid_to is null order by valid_to limit 1"
                , Database::query_param_list(date.is_special() ? boost::gregorian::day_clock::universal_day() : date.get() )
                );

        if(vat_details.size() > 1) {
            throw std::runtime_error("Multiple valid VAT values found.");
        } else if(vat_details.size() == 0) {
            throw std::runtime_error("No valid VAT value found.");
        }

        if(vat_details[0][0].isnull()) {
            vat_percent = 0;
        } else {
            vat_percent = vat_details[0][0];
        }

        if(vat_details[0][1].isnull()) {
            vat_reverse = 0;
        } else {
            vat_reverse = vat_details[0][1];
        }

        vat_amount = count_dph(price, vat_reverse);
        total = price - vat_amount;
    } else {
        total = price;
    }
    cent_amount credit = total;


    // get new invoice prefix and its type
    // TODO unclear thread safety in the old implementation
    Database::Result ip_res = conn.exec_params(
        "SELECT id, prefix  FROM invoice_prefix WHERE zone=$1::integer AND  typ=$2::integer AND year=$3::numeric FOR UPDATE",
        Database::query_param_list(zoneId)
                                (IT_DEPOSIT)
                                (date.year())
                                );

    if(ip_res.size() == 0 || ip_res[0][0].isnull() || ip_res[0][0].isnull()) {
        throw std::runtime_error("Missing invoice prefix");
    }

    int inv_prefix_type = ip_res[0][0];
    unsigned long long inv_prefix = ip_res[0][1];

    // increment invoice prefix in the DB
    conn.exec_params("UPDATE invoice_prefix SET prefix = $1::bigint WHERE id=$2::integer",
        Database::query_param_list(inv_prefix + 1)
                                  (inv_prefix_type));

    Database::Result curr_id = conn.exec("SELECT nextval('invoice_id_seq'::regclass)");
    if(curr_id.size() != 1 || curr_id[0][0].isnull()) {
        throw std::runtime_error("Couldn't fetch new invoice ID");
    }
    unsigned long long  invoiceId = curr_id[0][0];

    conn.exec_params(
            "INSERT INTO invoice (id, prefix, zone, prefix_type, registrarid, taxDate, price, vat, total, totalVAT, credit) VALUES "
            "($1::integer, $2::bigint, $3::integer, $4::integer, $5::integer, $6::date, $7::numeric(10,2), $8::integer, "
            "$9::numeric(10,2), $10::numeric(10,2), $11::numeric(10,2))", // total, totalVAT, credit
        Database::query_param_list(invoiceId)
                                (inv_prefix)
                                (zoneId)
                                (inv_prefix_type)
                                (registrarId)
                                (date)
                                (query_param_price(price))
                                (vat_percent)
                                (query_param_price(total))
                                (query_param_price(vat_amount))
                                (query_param_price(credit))
                                );

    return invoiceId;

}

int GetSystemVAT() // return VAT for invoicing depend on the time
{
    int dph=0;
    try
    {
        Database::Connection conn = Database::Manager::acquire();//get db conn
        Database::Result res = conn.exec(
            "select vat from price_vat where valid_to > now() or valid_to is null order by valid_to limit 1");

        if(res.size() == 1 && (res[0][0].isnull() == false))
        {
            dph = res[0][0];
        }
        else
            throw std::runtime_error("select vat from price_vat where valid_to > now() or valid_to is null order by valid_to limit 1 failed");
    }//try
    catch( std::exception &ex)
    {
        LOGGER(PACKAGE).error ( boost::format("GetSystemVAT failed: %1% ") % ex.what());
        throw std::runtime_error(std::string("GetSystemVAT failed: ") + ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("GetSystemVAT failed.");
        throw std::runtime_error("GetSystemVAT failed");
    }

    return dph;

}

long long GetInvoicePrefix(const std::string &dateStr, int typ, unsigned long long zone)
{
    long long prefix=0;
    try
    {
        int year = boost::lexical_cast<unsigned long>(dateStr.substr(0,4));
        unsigned long long id = 0;

        LOGGER(PACKAGE).debug ( boost::format("GetInvoicePrefix date[%1%]  year %2% typ %3% zone %4%\n")
            % dateStr % year % typ % zone);

        Database::Connection conn = Database::Manager::acquire();//get db conn

        Database::Result res = conn.exec_params(
            "SELECT id , prefix   FROM invoice_prefix WHERE zone=$1::bigint AND  typ=$2::integer AND year=$3::numeric FOR UPDATE"
            , Database::query_param_list (zone) (typ) (year));

        if(res.size() == 1 && (res[0][0].isnull() == false))
        {
            id = res[0][0];
            prefix = res[0][1];

            LOGGER(PACKAGE).debug ( boost::format("invoice_prefix id %1% -> %2%")
                % id % prefix);

            // increment invoice prefix in the DB
                conn.exec_params("UPDATE invoice_prefix SET prefix = $1::bigint WHERE id=$2::bigint"
                        , Database::query_param_list(prefix + 1)(id));
        }
        else
            throw std::runtime_error("SELECT id , prefix FROM invoice_prefix failed");
    }//try
    catch( std::exception &ex)
    {
        LOGGER(PACKAGE).error ( boost::format("GetInvoicePrefix failed: %1% ") % ex.what());
        throw std::runtime_error(std::string("GetInvoicePrefix failed: ") + ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("GetInvoicePrefix failed.");
        throw std::runtime_error("GetInvoicePrefix failed");
    }

    return prefix;
}

unsigned long long GetPrefixType(const std::string& dateStr, int typ, unsigned long long zone)
{
    unsigned long long id=0;
    try
    {
        int year = boost::lexical_cast<unsigned long>(dateStr.substr(0,4));

        LOGGER(PACKAGE).debug ( boost::format("GetPrefixType date[%1%]  year %2% typ %3% zone %4%\n")
                    % dateStr % year % typ % zone);

        Database::Connection conn = Database::Manager::acquire();//get db conn

        Database::Result res = conn.exec_params(
            "SELECT id  FROM invoice_prefix WHERE zone=$1::bigint AND  typ=$2::integer AND year=$3::numeric"
            , Database::query_param_list (zone) (typ) (year));
        if(res.size() == 1 && (res[0][0].isnull() == false))
        {
            id = res[0][0];
        }
        else
            throw std::runtime_error("SELECT id  FROM invoice_prefix failed");
    }//try
    catch( std::exception &ex)
    {
        LOGGER(PACKAGE).error ( boost::format("GetPrefixType failed: %1% ") % ex.what());
        throw std::runtime_error(std::string("GetPrefixType failed: ") + ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("GetPrefixType failed.");
        throw std::runtime_error("GetPrefixType failed");
    }

    return id;
}

// count new balance on advance invoice from total and all usages of that invoice
long GetInvoiceBalance(unsigned long long aID, long credit)
{
    long price=-1; // err value
    try
    {
        LOGGER(PACKAGE).debug ( boost::format("GetInvoiceBalance: zalohova FA %1%") % aID);

        Database::Connection conn = Database::Manager::acquire();//get db conn
        long total, suma;
        Database::Result res = conn.exec_params(
                "select total from invoice where id=$1::bigint"
                , Database::query_param_list(aID));

        if(res.size() == 1 && (res[0][0].isnull() == false))
        {
            total = (long) rint( 100.0 * atof(std::string(res[0][0]).c_str() ) );

            LOGGER(PACKAGE).debug ( boost::format("celkovy zaklad faktury %1%") % total);

            Database::Result res = conn.exec_params(
                "SELECT COALESCE( SUM( credit ), 0) FROM invoice_credit_payment_map WHERE ainvoiceid=$1::bigint"
                , Database::query_param_list(aID));
            if(res.size() == 1 && (res[0][0].isnull() == false))
            {
                suma = (long) rint( 100.0 * atof(std::string(res[0][0]).c_str() ) );
                LOGGER(PACKAGE).debug ( boost::format("sectweny credit %1%  pro zal FA") % suma);

                price = total - suma - credit;
                LOGGER(PACKAGE).debug ( boost::format("celkovy zustatek pri  uzavreni Fa %1%") % price);
            }
            else
                throw std::runtime_error("SELECT sum( credit ) FROM invoice_credit_payment_map failed");
        }
        else
            throw std::runtime_error("select total from invoice failed");

    }//try
    catch( std::exception &ex)
    {
        LOGGER(PACKAGE).error ( boost::format("GetInvoiceBalance failed: %1% ") % ex.what());
        throw std::runtime_error(std::string("GetInvoiceBalance failed: ") + ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("GetInvoiceBalance failed.");
        throw std::runtime_error("GetInvoiceBalance failed");
    }

    return price;
}

// return accounted price for invoice  iID from advance invoice  FA aID
long GetInvoiceSumaPrice(unsigned long long iID, unsigned long long aID)
{
    long price=-1; // err value
    try
    {
        LOGGER(PACKAGE).debug ( boost::format("GetInvoiceSumaPrice invoiceID %1% zalohova FA %2%")
        % iID % aID);
        Database::Connection conn = Database::Manager::acquire();//get db conn
        Database::Result res = conn.exec_params(
        "SELECT sum( invoice_object_registry_price_map.price ) FROM invoice_object_registry , invoice_object_registry_price_map "
        " WHERE invoice_object_registry.id=invoice_object_registry_price_map.id AND invoice_object_registry.invoiceid=$1::bigint AND "
        " invoice_object_registry_price_map.invoiceid=$2::bigint "
        , Database::query_param_list(iID) (aID));

        if(!res[0][0].isnull())
        {
            price = (long) rint( 100.0 * atof(std::string(res[0][0]).c_str() ) );
            LOGGER(PACKAGE).debug ( boost::format("celkovy strezeny credit z dane zal  Fa %1%") % price);
        }//if res not null

    }//try
    catch( std::exception &ex)
    {
        LOGGER(PACKAGE).error ( boost::format("GetInvoiceSumaPrice failed: %1% ") % ex.what());
        throw std::runtime_error(std::string("GetInvoiceSumaPrice failed: ") + ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("GetInvoiceSumaPrice failed.");
        throw std::runtime_error("GetInvoiceSumaPrice failed");
    }

    return price;
}

//returning invoiceID
unsigned long long  MakeNewInvoice(
  const std::string& taxDateStr, const std::string& fromdateStr, const std::string& todateStr,
  unsigned long long zone, unsigned long long  regID, long price, std::size_t count)
{
    unsigned long long invoiceID =0;
    try
    {
        LOGGER(PACKAGE).debug ( boost::format("MakeNewInvoice taxdate[%1%]  fromdateStr [%2%] todateStr[%3%]  zone %4% regID %5% , price %6%  count %7%")
        % taxDateStr % fromdateStr % todateStr % zone % regID % price % count);

        Database::Connection conn = Database::Manager::acquire();//get db conn

        unsigned long long type=0;
        long long prefix=0;
        int dph=0;

        if ( (type = GetPrefixType(taxDateStr, IT_ACCOUNT, zone) )) // usable prefix id of invoice
          {

            if (count) // create invoice
            {
              if ( (prefix = GetInvoicePrefix(taxDateStr, IT_ACCOUNT, zone) )) // number of invoice accord to taxable period
              {
                // find out VAT height
                dph =GetSystemVAT();

                LOGGER(PACKAGE).debug ( boost::format("Make Invoice prefix %1% type %2% DPH=%3%\n")
                % prefix % type % dph);

                invoiceID = conn.exec("SELECT NEXTVAL ('invoice_id_seq')")[0][0];

                conn.exec_params("INSERT INTO invoice "
                    " (id, prefix, zone, prefix_type "
                    " , registrarid, taxDate, price, vat"
                    " , total, totalVAT, credit) "
                    " VALUES ($1::bigint, $2::bigint, $3::bigint, $4::bigint"
                    ", $5::bigint, $6::date, $7::numeric, $8::integer"
                    ", $9::numeric, $10::integer, $11::numeric )"
                    , Database::query_param_list
                    (invoiceID)(prefix)(zone)(type)// link into prefix
                    (regID)(taxDateStr)
                    (query_param_price(price))// total price
                    (dph)// VAT is not null
                    (query_param_price(0)) // base without is zero amount
                    (0)
                    (Database::QPNull)// only credit is NULL
                    );

              }//if prefix
            }//if count
            else
	    {
                  invoiceID=0; // empty invoicing
	    }

            // record of invoicing
	    std::string recor_of_invoicing (
	    "INSERT INTO invoice_generation "
                " (fromdate, todate, registrarid, zone, invoiceID) "
                " VALUES ($1::date, $2::date, $3::bigint, $4::bigint, $5::bigint )" );
		
            conn.exec_params(recor_of_invoicing
                , Database::query_param_list
                (fromdateStr)(todateStr)(regID)(zone)
                (invoiceID ? Database::QueryParam(invoiceID) : Database::QPNull)
                );
	     
	     LOGGER(PACKAGE).debug ( boost::format(
	         "MakeNewInvoice record of invoicing sql: %1%  "
	         " $1: %2% $2: %3% $3: %4% $4: %5%  $5: %6% "
		 " count: %7% prefix: %8% invoiceID: %9% "
	     ) 
	        % recor_of_invoicing 
		% Database::QueryParam(fromdateStr).print_buffer()
		% Database::QueryParam(todateStr).print_buffer()
		% Database::QueryParam(regID).print_buffer()
		% Database::QueryParam(zone).print_buffer()
		% Database::QueryParam(invoiceID ? Database::QueryParam(invoiceID) : Database::QPNull).print_buffer()
		% count % prefix % invoiceID
	        );

          }//if prefix

    }//try
    catch( std::exception &ex)
    {
        LOGGER(PACKAGE).error ( boost::format("MakeNewInvoice failed: %1% ") % ex.what());
        throw std::runtime_error(std::string("MakeNewInvoice failed: ") + ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("MakeNewInvoice failed.");
        throw std::runtime_error("MakeNewInvoice failed");
    }

    return invoiceID;
}

//make new invoice returning invoiceID
unsigned long long MakeFactoring(unsigned long long regID
        , unsigned long long zone, const std::string& timestampStr
        , const std::string& taxDateStr)
{
    unsigned long long invoiceID =0;
    try
    {
        LOGGER(PACKAGE).debug ( boost::format("MakeFactoring regID %1% zone %2% timestampStr %3% taxDateStr %4%")
            % regID % zone % timestampStr % taxDateStr);

        Database::Connection conn = Database::Manager::acquire();//get db conn

        // last records todate plus one day in invoice_generation
        std::string fromdateStr;
        {
            Database::Result res = conn.exec_params(
            "SELECT date( todate + interval'1 day')  from invoice_generation "
            " WHERE zone=$1::bigint  AND registrarid =$2::bigint  order by id desc limit 1"
                    , Database::query_param_list(zone)(regID));
            if(res.size() == 1 && (res[0][0].isnull() == false))
                fromdateStr = std::string(res[0][0]);
        }

        if (fromdateStr.empty())
        {
            Database::Result res = conn.exec_params(
                "SELECT  fromdate  from registrarinvoice "
                " WHERE zone=$1::bigint and registrarid=$2::bigint"
                , Database::query_param_list(zone)(regID));
            if(res.size() == 1 && (res[0][0].isnull() == false))
                fromdateStr = std::string(res[0][0]);
        }

        std::string todateStr = boost::gregorian::to_iso_extended_string(
                (boost::posix_time::time_from_string(timestampStr)).date());

        LOGGER(PACKAGE).debug ( boost::format("MakeFactoring from %1% to %2% timestamp [%3%]")
            % fromdateStr % todateStr % timestampStr);

        // find out amount of item for invoicing
        std::size_t count = 0;
        {
            Database::Result res = conn.exec_params(
                "SELECT count( id)  from invoice_object_registry "
                " where crdate < $1::timestamp "
                " AND  zone=$2::bigint AND registrarid=$3::bigint AND invoiceid IS NULL"
                , Database::query_param_list(timestampStr)(zone)(regID));
            count = res[0][0];
        }

        // find out total invoiced price if it exists al least one record
        long price = 0;
        if (count > 0)
        {
            Database::Result res = conn.exec_params(
                "SELECT sum( price ) "
                " FROM invoice_object_registry , invoice_object_registry_price_map  "
                " WHERE invoice_object_registry_price_map.id=invoice_object_registry.id "
                " AND crdate < $1::timestamp AND zone=$2::bigint and registrarid=$3::bigint "
                " AND  invoice_object_registry.invoiceid is null"
                , Database::query_param_list(timestampStr)(zone)(regID));
            price = (long) rint( 100.0 * atof(std::string(res[0][0]).c_str() ) );

            LOGGER(PACKAGE).debug ( boost::format("Total price %1%")
                % price);
        }

        // empty invoice invoicing record
        // returns invoiceID or null if nothings was invoiced, on error returns negative number of error
        if ( (invoiceID = MakeNewInvoice(taxDateStr, fromdateStr, todateStr, zone,
            regID, price, count) ) >= 0)
        {
            if (count > 0) // mark item of invoice
            {
                conn.exec_params(
                "UPDATE invoice_object_registry set invoiceid=$1::bigint "
                " WHERE crdate < $2::timestamp AND zone=$3::bigint  "
                " and registrarid=$4::bigint AND invoiceid IS NULL"
                , Database::query_param_list
                    (invoiceID)(timestampStr)(zone)(regID));

            }

            // set last date into tabel registrarinvoice
            conn.exec_params(
                "UPDATE registrarinvoice SET lastdate=$1::date "
                " WHERE zone=$2::bigint and registrarid=$3::bigint"
                , Database::query_param_list
                (todateStr)(zone)(regID));

            // if invoice was created
            if (invoiceID > 0)
            {
                // query for all advance invoices, from which were gathered for taxes FA
                Database::Result res = conn.exec_params(
                    "select invoice_object_registry_price_map.invoiceid "
                    " from  invoice_object_registry ,  invoice_object_registry_price_map "
                    " where invoice_object_registry.id=invoice_object_registry_price_map.id "
                    "  and invoice_object_registry.invoiceid=$1::bigint "
                    " GROUP BY invoice_object_registry_price_map.invoiceid"
                    , Database::query_param_list(invoiceID));

                for (std::size_t i = 0; i < res.size(); ++i)
                {
                    // insert into table invoice_credit_payment_map;
                    unsigned long long aID = res[i][0];
                    LOGGER(PACKAGE).debug ( boost::format("zalohova FA -> %1%")
                                                % aID);
                    long credit = GetInvoiceSumaPrice(invoiceID, aID);
                    long balance = GetInvoiceBalance(aID, credit); // actual available balance
                    if (balance >=0)
                    {
                        LOGGER(PACKAGE).debug ( boost::format("zalohova FA  %1% credit %2% balance %3%")
                            % aID % credit % balance);

                        conn.exec_params("INSERT INTO invoice_credit_payment_map "
                          " (invoiceid, ainvoiceid, credit, balance) "
                          " VALUES ($1::bigint, $2::bigint, $3::numeric, $4::numeric)"
                          , Database::query_param_list
                          (invoiceID)(aID)
                          (query_param_price(credit))
                          (query_param_price(balance))
                      );

                    }//if balance >=0

                }//for i res.size()

            }//if invoiceID > 0

        }//if invoiceID >=0

    }//try
    catch( std::exception &ex)
    {
        LOGGER(PACKAGE).error ( boost::format("MakeFactoring failed: %1% ") % ex.what());
        throw std::runtime_error(std::string("MakeFactoring failed: ") + ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("MakeFactoring failed.");
        throw std::runtime_error("MakeFactoring failed");
    }

    return invoiceID;
}

void createAccountInvoices( const std::string& zone_fqdn, const std::string& taxdateStr, const std::string& todateStr)
{
    Logging::Context ctx("createAccountInvoices");
    try
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        std::string  timestampStr = todateStr;

        Database::Result res = conn.exec_params(
          "SELECT r.id, z.id FROM registrar r, registrarinvoice i, zone z WHERE r.id=i.registrarid"
          " AND r.system=false AND i.zone=z.id AND z.fqdn=$1::text"
          " AND i.fromdate<=CURRENT_DATE"
          , Database::query_param_list (zone_fqdn)
          );

        LOGGER(PACKAGE).debug ( boost::format("ManagerImpl::createAccountInvoices"
                " zone_fqdn %1%  taxdateStr %2% todateStr %3%")
        % zone_fqdn % taxdateStr % todateStr);


        for(std::size_t i = 0; i < res.size(); ++i)
        {
            unsigned long long regID = res[i][0];//regID
            unsigned long long zoneID =res[i][1];//zoneID

            unsigned long long invoiceID = MakeFactoring(regID, zoneID, timestampStr,taxdateStr);
            LOGGER(PACKAGE).notice(boost::format(
             "Vygenerovana fa invoiceID %1% pro regID %2% zoneID %3% timestampStr %4% taxdateStr %5%")
             % invoiceID % regID % zoneID % timestampStr % taxdateStr);
        }//for i

        tx.commit();
    }//try
    catch( std::exception &ex)
    {
        LOGGER(PACKAGE).error ( boost::format("createAccountInvoices failed: %1% ") % ex.what());
        throw std::runtime_error(std::string("createAccountInvoices failed: ") + ex.what());
    }
    catch(...)
    {
        LOGGER(PACKAGE).error("createAccountInvoices failed.");
        throw std::runtime_error("createAccountInvoices failed");
    }
}//createAccountInvoices

// close invoice to registar handle for zone make taxDate to the todateStr
void createAccountInvoice( const std::string& registrarHandle, const std::string& zone_fqdn, const std::string& taxdateStr, const std::string& todateStr)
{
    Logging::Context ctx("createAccountInvoice");
    try
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        std::string timestampStr;
        unsigned long long regID = 0;
        unsigned long long zone = 0;

        Database::Result res = conn.exec_params(
            "SELECT id FROM registrar WHERE handle=$1::text"
            , Database::query_param_list (registrarHandle));

        if(res.size() == 1 && (res[0][0].isnull() == false))
        {
            regID = res[0][0];//regID

            Database::Result res = conn.exec_params(
                "SELECT id FROM zone WHERE fqdn=$1::text"
                , Database::query_param_list (zone_fqdn));

            if(res.size() == 1 && (res[0][0].isnull() == false))
            {
                zone = res[0][0];//zone

                std::string  timestampStr = todateStr;

                LOGGER(PACKAGE).debug ( boost::format("ManagerImpl::createAccountInvoice"
                        " regID %1%  zone %2% taxdateStr %3%  timestampStr %4%")
                % regID % zone % taxdateStr % timestampStr );

                // make invoice
                MakeFactoring(regID, zone, timestampStr, taxdateStr);

            }
            else
            {
                std::string msg = str( boost::format("unknown zone %1% \n") % zone_fqdn);
                LOGGER(PACKAGE).error(msg  );
                throw std::runtime_error(msg);
            }

        }
        else
        {
            std::string msg = str( boost::format("unknown registrarHandle %1% ") % registrarHandle );
            LOGGER(PACKAGE).error( msg );
            throw std::runtime_error(msg);
        }

        tx.commit();
    }//try
    catch( std::exception &ex)
    {
      LOGGER(PACKAGE).error ( boost::format("createAccountInvoice failed: %1% ") % ex.what());
      throw std::runtime_error(std::string("createAccountInvoice failed: ") + ex.what());
    }
    catch(...)
    {
      LOGGER(PACKAGE).error("createAccountInvoice failed.");
      throw std::runtime_error("createAccountInvoice failed");
    }

}//createAccountInvoice

}; // ManagerImpl

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//   SubjecImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/// implementation of Subject interface 
class SubjectImpl : public Subject {
  TID id;
  std::string handle;
  std::string name;
  std::string fullname;
  std::string street;
  std::string city;
  std::string zip;
  std::string country;
  std::string ico;
  std::string vatNumber;
  std::string registration;
  std::string reclamation;
  std::string email;
  std::string url;
  std::string phone;
  std::string fax;
  bool vatApply;
public:
  SubjectImpl(TID _id,
              const std::string& _handle,
              const std::string& _name,
              const std::string& _fullname,
              const std::string& _street,
              const std::string& _city,
              const std::string& _zip,
              const std::string& _country,
              const std::string& _ico,
              const std::string& _vatNumber,
              const std::string& _registration,
              const std::string& _reclamation,
              const std::string& _email,
              const std::string& _url,
              const std::string& _phone,
              const std::string& _fax,
              bool _vatApply) :
    id(_id), handle(_handle), name(_name), fullname(_fullname),
        street(_street), city(_city), zip(_zip), country(_country), ico(_ico),
        vatNumber(_vatNumber), registration(_registration),
        reclamation(_reclamation), email(_email), url(_url), phone(_phone),
        fax(_fax), vatApply(_vatApply) {
  }
  TID getId() const {
    return id;
  }
  const std::string& getHandle() const {
    return handle;
  }
  const std::string& getName() const {
    return name;
  }
  const std::string& getFullname() const {
    return fullname;
  }
  const std::string& getStreet() const {
    return street;
  }
  const std::string& getCity() const {
    return city;
  }
  const std::string& getZip() const {
    return zip;
  }
  const std::string& getCountry() const {
    return country;
  }
  const std::string& getICO() const {
    return ico;
  }
  const std::string& getVatNumber() const {
    return vatNumber;
  }
  bool getVatApply() const {
    return vatApply;
  }
  const std::string& getRegistration() const {
    return registration;
  }
  const std::string& getReclamation() const {
    return reclamation;
  }
  const std::string& getEmail() const {
    return email;
  }
  const std::string& getURL() const {
    return url;
  }
  const std::string& getPhone() const {
    return phone;
  }
  const std::string& getFax() const {
    return fax;
  }
};
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//   PaymentImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/// implementation of Payment interface
class PaymentImpl : virtual public Payment {
  Money price; ///< money that come from this advance invoice  
  unsigned vatRate; ///< vatRate of this advance invoice
  Money vat; ///< total vat - approx. (price * vatRate/100)
public:
  PaymentImpl(const PaymentImpl* p) :
    price(p->price), vatRate(p->vatRate), vat(p->vat) {
  }
  PaymentImpl(Money _price, unsigned _vatRate, Money _vat) :
    price(_price), vatRate(_vatRate), vat(_vat) {
  }
  virtual Money getPrice() const {
    return price;
  }
  virtual unsigned getVatRate() const {
    return vatRate;
  }
  virtual Money getVat() const {
    return vat;
  }
  virtual Money getPriceWithVat() const {
    return price + vat;
  }
  bool operator==(unsigned _vatRate) const {
    return vatRate == _vatRate;
  }
  void add(const PaymentImpl *p) {
    price += p->price;
    vat += p->vat;
  }
};
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//   PaymentSourceImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/// implementation of PaymentSource interface
class PaymentSourceImpl : public PaymentImpl, virtual public PaymentSource {
  unsigned long long number; ///< number of source advance invoice
  Money credit; ///< credit remaining on this advance invoice 
  TID id; ///< id of source advance invoice
  Money totalPrice; ///< total price without vat on advance invoice
  Money totalVat; ///< total vat on advance invoice
  ptime crtime; ///< creation time of advance invoice 
public:
  /// init content from sql result (ignore first column)
  PaymentSourceImpl(Money _price, unsigned _vat_rate, Money _vat, 
                    unsigned long long _number, Money _credit, TID _id,
                    Money _total_price, Money _total_vat, ptime _crtime) :
                      PaymentImpl(_price, _vat_rate, _vat),
                      number(_number),
                      credit(_credit),
                      id(_id),
                      totalPrice(_total_price),
                      totalVat(_total_vat),
                      crtime(_crtime) {    
  }
  
  virtual unsigned long long getNumber() const {
    return number;
  }
  virtual Money getCredit() const {
    return credit;
  }
  virtual TID getId() const {
    return id;
  }
  virtual Money getTotalPrice() const {
    return totalPrice;
  }
  virtual Money getTotalVat() const {
    return totalVat;
  }
  virtual Money getTotalPriceWithVat() const {
    return totalPrice + totalVat;
  }
  virtual ptime getCrTime() const {
    return crtime;
  }

};
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//   PaymentActionImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/// implementation of PaymentAction interface
class PaymentActionImpl : public PaymentImpl, virtual public PaymentAction {
  std::string objectName; ///< name of object affected by payment action
  ptime actionTime; ///< time of payment action
  date exDate; ///< exdate of domain 
  PaymentActionType action; ///< type of action that is subject of payment
  unsigned unitsCount; ///< number of months to expiration of domain
  Money pricePerUnit; ///< copy of price from price list
  TID objectId; ///< id of object affected by payment action
public:
  /// init content from sql result (ignore first column)
  PaymentActionImpl(Money _price, unsigned _vat_rate, Money _vat, 
                    std::string& _object_name, ptime _action_time, date _exdate,
                    PaymentActionType _type, unsigned _units, Money _price_per_unit, TID _id) :
                      PaymentImpl(_price, _vat_rate, _vat),
                      objectName(_object_name),
                      actionTime(_action_time),
                      exDate(_exdate),
                      action(_type),
                      unitsCount(_units),
                      pricePerUnit(_price_per_unit),
                      objectId(_id) {                        
  }
  
  virtual TID getObjectId() const {
    return objectId;
  }
  virtual const std::string& getObjectName() const {
    return objectName;
  }
  virtual ptime getActionTime() const {
    return actionTime;
  }
  virtual date getExDate() const {
    return exDate;
  }
  virtual PaymentActionType getAction() const {
    return action;
  }
  virtual unsigned getUnitsCount() const {
    return unitsCount;
  }
  virtual Money getPricePerUnit() const {
    return pricePerUnit;
  }
  virtual std::string getActionStr() const
    {        
        switch (getAction()) {
            case PAT_CREATE_DOMAIN:
                return "CREATE";
                break;
            case PAT_RENEW_DOMAIN:
                return "RENEW";
                break;
            default:
                return "UNKNOWN";
                break;
        }

        return "UNKNOWN";
    }
};
/// hold list of sum of proportional parts of prices for every year
class AnnualPartitioningImpl : public virtual AnnualPartitioning {
  /// type for mapping year to sum of money
  typedef std::map<unsigned, Money> RecordsType;
  RecordsType::const_iterator i; ///< for walkthrough in results 
  typedef std::map<unsigned, RecordsType> vatRatesRecordsType;
  vatRatesRecordsType::const_iterator j; ///< for walkthrough in results 
  vatRatesRecordsType records; ///< list of years by vat rate
  ManagerImpl *man; ///< need to count vat
  bool noVatRate; ///< there is not vat rate asked in resetIterator()
public:
  AnnualPartitioningImpl(ManagerImpl* _man) :
    man(_man), noVatRate(true) {
  }
  /** for every year in period from exdate-unitsCount to exdate 
   * count proportional part of price according to days that belong 
   * to relevant year */
  /// partition action prices into years
  void addAction(PaymentAction *pa) {
    // non periodical actions are ignored
    if (!pa->getUnitsCount() || pa->getExDate().is_special())
      return;
    // lastdate will be subtracted down in every iteration
    date lastdate = pa->getExDate();
    // firstdate is for detection when to stop and for portion counting
    date firstdate = pa->getExDate() - months(pa->getUnitsCount());
    // money that still need to be partitioned
    Money remains = pa->getPrice();
    while (remains) {
      Money part;
      unsigned year = lastdate.year();
      if (year == firstdate.year())
        // last year just take what remains
        part = remains;
      else {
        // count portion of remains and update lastdate
        date newdate = date(year, 1, 1) - days(1);
        part = remains * (lastdate - newdate).days() / (lastdate - firstdate).days();
        lastdate = newdate;
      }
      remains -= part;
      records[pa->getVatRate()][year] += part;
    }
  }
  void resetIterator(unsigned vatRate) {
    j = records.find(vatRate);
    if (j == records.end())
      noVatRate = true;
    else {
      noVatRate = false;
      i = j->second.begin();
    }
  }
  bool end() const {
    return noVatRate || i == j->second.end();
  }
  void next() {
    i++;
  }
  unsigned getYear() const {
    return end() ? 0 : i->first;
  }
  Money getPrice() const {
    return end() ? 0 : i->second;
  }
  unsigned getVatRate() const {
    return end() ? 0 : j->first;
  }
  Money getVat() const {
    return man->countVAT(getPrice(), getVatRate(), true);
  }
  Money getPriceWithVat() const {
    return getPrice() + getVat();
  }
};
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//   Exporter
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/// common exporter interface 
class Exporter {
public:
  virtual ~Exporter() {
  }
  virtual void doExport(Invoice *) = 0;
};
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//   InvoiceImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/// implementation of interface Invoice


class InvoiceImpl : public Fred::CommonObjectImpl,
                    virtual public Invoice {
  DB *dbc;
  TID zone;
  std::string zoneName;
  ptime crTime;
  date taxDate;
  date_period accountPeriod;
  Type type;
  unsigned long long number;
  TID registrar;
  Money credit;
  Money price;
  short vatRate;
  Money total;
  Money totalVAT;
  TID filePDF;
  TID fileXML;
  std::string varSymbol;
  SubjectImpl client;
  std::string filepdf_name;
  std::string filexml_name;
  static SubjectImpl supplier;
  std::vector<PaymentSourceImpl *> sources;
  std::vector<PaymentActionImpl *> actions;
  bool storeFileFlag; ///< ready for saving link to generated file
  AnnualPartitioningImpl ap; ///< total prices partitioned by year
  std::vector<PaymentImpl> paid; ///< list of paid vat rates
  ManagerImpl *man; ///< backlink to manager for VAT and others
  TID id;

  void clearLists() {
    for (unsigned i=0; i<sources.size(); i++)
      delete sources[i];
    for (unsigned i=0; i<actions.size(); i++)
      delete actions[i];
  }
public:
  
  InvoiceImpl(TID _id, TID _zone, std::string& _zoneName, ptime _crTime, date _taxDate,
              date_period& _accountPeriod, Type _type, unsigned long long _number,
              TID _registrar, Money _credit, Money _price, short _vatRate, Money _total,
              Money _totalVAT, TID _filePDF, TID _fileXML, std::string& _varSymbol,
              SubjectImpl& _client, const std::string &_filepdf_name, const std::string &_filexml_name,
              ManagerImpl *_manager) : CommonObjectImpl(_id),
                                       dbc(0),
                                       zone(_zone),
                                       zoneName(_zoneName),
                                       crTime(_crTime),
                                       taxDate(_taxDate),
                                       accountPeriod(_accountPeriod),
                                       type(_type),
                                       number(_number),
                                       registrar(_registrar),
                                       credit(_credit),
                                       price(_price),
                                       vatRate(_vatRate),
                                       total(_total),
                                       totalVAT(_totalVAT),
                                       filePDF(_filePDF),
                                       fileXML(_fileXML),
                                       varSymbol(_varSymbol),
                                       client(_client),
                                       filepdf_name(_filepdf_name),
                                       filexml_name(_filexml_name),
                                       storeFileFlag(false),
                                       ap(_manager),
                                       man(_manager),
                                       id(_id) {
      LOGGER(PACKAGE).debug ( boost::format(
              "InvoiceImpl _id: %1% _zone: %2% _zoneName: %3% _crTime: %4% _taxDate: %5%"
              " _accountPeriod: %6% _type: %7% _number: %8%"
              " _registrar: %9% _credit: %10% _price: %11% _vatRate: %12% _total: %13%"
              " _totalVAT: %14% _filePDF: %15% _fileXML: %16% _varSymbol: %17%"
              //" _client: %18% _filepdf_name: %19%  _filexml_name: %20%"
              //" _manager: %21%"
              )
      % _id % _zone % _zoneName % _crTime % _taxDate
      % _accountPeriod % _type %  _number
      % _registrar % _credit % _price % _vatRate % _total
      % _totalVAT % _filePDF % _fileXML % _varSymbol
      //% _client % _filepdf_name % _filexml_name
      //% _manager

      );
  }

  ~InvoiceImpl() {
    clearLists();
  }
  const Subject* getClient() const {
    return &client;
  }
  const Subject* getSupplier() const {
    return &supplier;
  }
  TID getZone() const {
    return zone;
  }
  const std::string& getZoneName() const {
    return zoneName;
  }
  ptime getCrTime() const {
    return crTime;
  }
  // new interface
  ptime getCrDate() const {
    return getCrTime();
  }
  date getTaxDate() const {
    return taxDate;
  }
  date_period getAccountPeriod() const {
    return accountPeriod;
  }
  Type getType() const {
    return type;
  }
  unsigned long long getNumber() const {
    return number;
  }
  TID getRegistrar() const {
    return registrar;
  }
  TID getRegistrarId() const {
    return getRegistrar();
  }
  Money getCredit() const {
    return credit;
  }
  Money getPrice() const {
    return price;
  }
  short getVatRate() const {
    return vatRate;
  }
  Money getTotal() const {
    return total;
  }
  Money getTotalVAT() const {
    return totalVAT;
  }
  const std::string& getVarSymbol() const {
    return varSymbol;
  }
  TID getFilePDF() const {
    return filePDF;
  }
  TID getFileXML() const {
    return fileXML;
  }
  std::string getFileNamePDF() const {
    return filepdf_name;
  }
  std::string getFileNameXML() const {
    return filexml_name;
  }
  unsigned getSourceCount() const {
    return sources.size();
  }
  const PaymentSource *getSource(unsigned idx) const {
    return idx>=sources.size() ? NULL : sources[idx];
  }
  unsigned getActionCount() const {
    return actions.size();
  }
  const PaymentAction *getAction(unsigned idx) const {
    return idx>=actions.size() ? NULL : actions[idx];
  }
  void setFile(TID _filePDF, TID _fileXML) {
    // set only once (disabling overwrite link to archived file)
    if (!filePDF) {
      filePDF = _filePDF;
      fileXML = _fileXML;
      // intention was separate setting of file id and it's storage
      // outside of document generation process. temporary 
      // combined into setting function
      storeFileFlag = true;
      try {
        storeFile();
      }
      catch (...) {}
    }
  }
  void storeFile()
  {
    // cannot rollback generated files so ignoring if XML file hasn't
    // been generated

      Logging::Context ctx("storeFile");
      try
      {
          if (storeFileFlag && filePDF)
          {
              Database::Connection conn = Database::Manager::acquire();
              Database::QueryParams sql_params;//query params
              std::stringstream sql;

              sql_params.push_back(boost::lexical_cast<std::string>(filePDF));
              sql << "UPDATE invoice SET file=$"<< sql_params.size() <<"::bigint ";

              if (fileXML)
              {
                  sql_params.push_back(boost::lexical_cast<std::string>(fileXML));
                  sql << ",fileXML=$"<< sql_params.size() <<"::bigint ";
              }

              sql_params.push_back(boost::lexical_cast<std::string>(getId()));
              sql << " WHERE id=$"<< sql_params.size() <<"::bigint ";

              conn.exec_params(sql.str(), sql_params);
          }

      }//try
      catch( std::exception &ex)
      {
          LOGGER(PACKAGE).error ( boost::format("storeFile failed: %1% ") % ex.what());
          throw std::runtime_error(std::string("storeFile failed: ") + ex.what());
      }
      catch(...)
      {
          LOGGER(PACKAGE).error("storeFile failed.");
          throw std::runtime_error("storeFile failed");
      }
  }//storeFile

  /// export invoice using given exporter
  void doExport(Exporter *exp) {
    exp->doExport(this);
  }
  /// initialize list of actions from sql result
   
  void addAction(Database::Row::Iterator& _col) {
    Database::Money    price       = *_col;
    unsigned           vat_rate    = *(++_col);
    std::string        object_name = *(++_col);
    Database::DateTime action_time = *(++_col); 
    Database::Date     exdate      = *(++_col);
    PaymentActionType  type        = (int)*(++_col) == 1 ? PAT_CREATE_DOMAIN 
                                                         : PAT_RENEW_DOMAIN;
    unsigned           units          = *(++_col); 
    Database::Money    price_per_unit = *(++_col);
    Database::ID       id             = *(++_col);
                          
    PaymentActionImpl *new_action = new PaymentActionImpl(price,
                                                          vat_rate,
                                                          man->countVAT(price, vat_rate, true),
                                                          object_name,
                                                          action_time,
                                                          exdate,
                                                          type,
                                                          units,
                                                          price_per_unit,
                                                          id);
    actions.push_back(new_action);
    ap.addAction(actions.back());
  }
  /// initialize list of sources from sql result

  void addSource(Database::Row::Iterator& _col) {
    Database::Money price       = *_col;
    unsigned vat_rate           = *(++_col);
    unsigned long long number   = *(++_col);  
    Database::Money credit      = *(++_col);
    Database::ID id             = *(++_col);
    Database::Money total_price = *(++_col);
    Database::Money total_vat   = *(++_col);
    Database::DateTime crtime   = *(++_col);
    
    PaymentSourceImpl *new_source = new PaymentSourceImpl(price,
                                                          vat_rate,
                                                          man->countVAT(price, vat_rate, true),
                                                          number,
                                                          credit,
                                                          id,
                                                          total_price,
                                                          total_vat,
                                                          crtime);
    sources.push_back(new_source);
    
    // init vat groups, if vat rate exists, add it, otherwise create new
    std::vector<PaymentImpl>::iterator i = find(paid.begin(),
                                                paid.end(),
                                                new_source->getVatRate() );
    if (i != paid.end())
      i->add(new_source);
    else
      paid.push_back(PaymentImpl(new_source));    
  }
  
  virtual AnnualPartitioning *getAnnualPartitioning() {
    return &ap;
  }
  virtual unsigned getPaymentCount() const {
    // virtualize advance payment into list of payments to
    // provide single point of data
    // overcasting to stay const
    if (type == IT_DEPOSIT && !paid.size())
      ((InvoiceImpl *)this)->paid.push_back(PaymentImpl(getTotal(),
                                                              getVatRate(),
                                                              getTotalVAT()) );
    return paid.size();
  }
  virtual const Payment *getPaymentByIdx(unsigned idx) const {
    return idx >= paid.size() ? NULL : &paid[idx];
  }
   
    virtual  TID getZoneId() const {
        return getZone();
    };
    
    virtual  TID getPrefix() const {
        return getNumber();
    };
    virtual int getVat() const {
        return getVatRate();
    };
    virtual const Database::Money getTotalVat() const {
        return getTotalVAT();
    };    
    virtual  TID getFileId() const {
        return getFilePDF();
    };
    virtual  std::string getFileHandle() const {
        return getFileNamePDF();
    };
    virtual  TID getFileXmlId() const {
        return getFileXML();
    };
    virtual  std::string getFileXmlHandle() const {
        return getFileNameXML();
    };
    
    virtual std::string getZoneFqdn() const {
        return getZoneName();
    };
  
    virtual const Payment *getPayment(const unsigned int &index) const {
        return (const Payment*)getAction(index); 
    };

  virtual bool save() {
        return true;
  };
 
};


// TODO: should be initalized somewhere else
/// static supplier in every invoice
SubjectImpl
    InvoiceImpl::supplier( 0,
                          "REG-CZNIC",
                          "CZ.NIC, z.s.p.o.",
                          "CZ.NIC, zájmové sdružení právnických osob",
                          "Americká 23",
                          "Praha 2",
                          "120 00",
                          "CZ",
                          "67985726",
                          "CZ67985726",
                          "SpZ: odb. občanskopr. agend Magist. hl. m. Prahy, č. ZS/30/3/98",
                          "CZ.NIC, z.s.p.o., Americká 23, 120 00 Praha 2",
                          "www.nic.cz",
                          "podpora@nic.cz",
                          "+420 222 745 111",
                          "+420 222 745 112",
                           1);
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//   ExporterXML
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#define TAGSTART(tag) "<"#tag">"
#define TAGEND(tag) "</"#tag">"
#define TAG(tag,f) TAGSTART(tag) \
                       << "<![CDATA[" << f << "]]>" << TAGEND(tag)
#define OUTMONEY(f) (f)/100 << "." << \
                        std::setfill('0') << std::setw(2) << abs(f)%100
// builder that export xml of invoice into given stream
class ExporterXML : public Exporter {
  std::ostream& out;
  bool xmlDec; ///< whether to include xml declaration 
public:
  ExporterXML(std::ostream& _out, bool _xmlDec) :
    out(_out), xmlDec(_xmlDec) {
  }
  std::ostream& doExport(const Subject* s)
  {
    if(!s) throw std::runtime_error("ExporterXML::doExport s");
    out << TAG(id,s->getId())
        << TAG(name,s->getName())
        << TAG(fullname,s->getFullname())
        << TAGSTART(address)
        << TAG(street,s->getStreet())
        << TAG(city,s->getCity())
        << TAG(zip,s->getZip())
        << TAG(country,s->getCountry())
        << TAGEND(address)
        << TAG(ico,s->getICO())
        << TAG(vat_number,s->getVatNumber())
        << TAG(registration,s->getRegistration())
        << TAG(reclamation,s->getReclamation())
        << TAG(url,s->getURL())
        << TAG(email,s->getEmail())
        << TAG(phone,s->getPhone())
        << TAG(fax,s->getFax())
        << TAG(vat_not_apply,(s->getVatApply() ? 0 : 1));
    return out;
  }
    virtual void doExport(Invoice *i)
    {
        if(!i) throw std::runtime_error("ExporterXML::doExport i");
      // setting locale for proper date and time format
      // do not use system locale - locale("") because of
      // unpredictable formatting behavior
      out.imbue(std::locale(
              std::locale(
                  out.getloc(),
                  new time_facet("%Y-%m-%d %T")
              ),
              new date_facet("%Y-%m-%d")
          ));
      // generate invoice xml
      if (xmlDec)
      out << "<?xml version='1.0' encoding='utf-8'?>";
      out << TAGSTART(invoice)
      << TAGSTART(client);
      doExport(i->getClient());
      out << TAGEND(client)
      << TAGSTART(supplier);
      doExport(i->getSupplier());
      out << TAGEND(supplier)
      << TAGSTART(payment)
      << TAG(invoice_number,i->getNumber())
      << TAG(invoice_date,i->getCrTime().date());
      if (i->getType() == IT_DEPOSIT)
      out << TAG(advance_payment_date,i->getTaxDate());
      else {
        out << TAG(tax_point,i->getTaxDate())
        << TAG(period_from,i->getAccountPeriod().begin())
        << TAG(period_to,i->getAccountPeriod().end());
      }
      out << TAG(vs,i->getVarSymbol())
      << TAGEND(payment)
      << TAGSTART(delivery)
      << TAGSTART(vat_rates);
      for (unsigned j=0; j<i->getPaymentCount(); j++) {
        const Payment *p = i->getPaymentByIdx(j);
        out << TAGSTART(entry)
        << TAG(vatperc,p->getVatRate())
        << TAG(basetax,OUTMONEY(p->getPrice()))
        << TAG(vat,OUTMONEY(p->getVat()))
        << TAG(total,OUTMONEY(p->getPriceWithVat()))
        << TAGSTART(years);
        for (
            i->getAnnualPartitioning()->resetIterator(p->getVatRate());
            !i->getAnnualPartitioning()->end();
            i->getAnnualPartitioning()->next()
        ) {
          AnnualPartitioning *ap = i->getAnnualPartitioning();
          out << TAGSTART(entry)
          << TAG(year,ap->getYear())
          << TAG(price,OUTMONEY(ap->getPrice()))
          << TAG(vat,OUTMONEY(ap->getVat()))
          << TAG(total,OUTMONEY(ap->getPriceWithVat()))
          << TAGEND(entry);
        }
        out << TAGEND(years)
        << TAGEND(entry);
      }
      out << TAGEND(vat_rates)
      << TAGSTART(sumarize)
      << TAG(total,OUTMONEY(i->getPrice()))
      << TAG(paid,
          OUTMONEY((i->getType() != IT_DEPOSIT ? -i->getPrice() : 0)))
      << TAG(to_be_paid,OUTMONEY(0))
      << TAGEND(sumarize)
      << TAGEND(delivery);
      if (i->getSourceCount()) {
        out << TAGSTART(advance_payment)
        << TAGSTART(applied_invoices);
        for (unsigned k=0; k<i->getSourceCount(); k++) {
          const PaymentSource *ps = i->getSource(k);
          out << TAGSTART(consumed)
          << TAG(number,ps->getNumber())
          << TAG(price,OUTMONEY(ps->getPrice()))
          << TAG(balance,OUTMONEY(ps->getCredit()))
          << TAG(vat,OUTMONEY(ps->getVat()))
          << TAG(vat_rate,ps->getVatRate())
          << TAG(pricevat,OUTMONEY(ps->getPriceWithVat()))
          << TAG(total,OUTMONEY(ps->getTotalPrice()))
          << TAG(total_vat,OUTMONEY(ps->getTotalVat()))
          << TAG(total_with_vat,OUTMONEY(ps->getTotalPriceWithVat()))
          << TAG(crtime,ps->getCrTime())
          << TAGEND(consumed);
        }
        out << TAGEND(applied_invoices)
        << TAGEND(advance_payment);
      }
      if (i->getActionCount()) {
        out << TAGSTART(appendix)
        << TAGSTART(items);
        for (unsigned k=0; k<i->getActionCount(); k++) {
          const PaymentAction *pa = i->getAction(k);
          out << TAGSTART(item)
          << TAG(subject,pa->getObjectName())
          << TAG(code,
              (pa->getAction() == PAT_CREATE_DOMAIN ?
                  "RREG" : "RUDR"))
          << TAG(timestamp,pa->getActionTime());
          if (!pa->getExDate().is_special())
          out << TAG(expiration,pa->getExDate());
          out << TAG(count,pa->getUnitsCount()/12) // in years
          << TAG(price,OUTMONEY(pa->getPricePerUnit()))
          << TAG(total,OUTMONEY(pa->getPrice()))
          << TAG(vat_rate,pa->getVatRate())
          << TAGEND(item);
        }
        out << TAGEND(items)
        << TAGSTART(sumarize_items)
        << TAG(total,OUTMONEY(i->getPrice()))
        << TAGEND(sumarize_items)
        << TAGEND(appendix);
      }
      out << TAGEND(invoice);
    }
  };
#define INVOICE_PDF_FILE_TYPE 1 
#define INVOICE_XML_FILE_TYPE 2 
  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //   ExporterArchiver
  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  // builder that create pdf of invoice and store it into filesystem
  class ExporterArchiver : public Exporter
  {
    Document::Manager *docman;
    std::string makeFileName(Invoice *i, const char *suffix)
    {
      std::stringstream filename;
      filename << i->getNumber() << suffix;
      return filename.str();
    }
  public:
    ExporterArchiver(Document::Manager *_docman) : docman(_docman) {}
    virtual void doExport(Invoice *i)
    {
      if(!i) throw std::runtime_error("ExporterArchiver::doExport i");
      try {
        // create generator for pdf 
        std::auto_ptr<Document::Generator> gPDF(
            docman->createSavingGenerator(
                i->getType() == IT_DEPOSIT ?
                Document::GT_ADVANCE_INVOICE_PDF :
                Document::GT_INVOICE_PDF,
                makeFileName(i,".pdf"),INVOICE_PDF_FILE_TYPE,
                i->getClient()->getCountry() == "CZ" ? "cs" : "en"
            )
        );
        // feed generator with xml input using xml exporter
        ExporterXML(gPDF->getInput(),true).doExport(i);
        // return id of generated PDF file
        TID filePDF = gPDF->closeInput();

        LOGGER(PACKAGE).debug ( boost::format("ExporterArchiver::doExport pdf file id: %1% ") % filePDF);

        // create generator for XML
        std::auto_ptr<Document::Generator> gXML(
            docman->createSavingGenerator(
                Document::GT_INVOICE_OUT_XML,
                makeFileName(i,".xml"),INVOICE_XML_FILE_TYPE,
                i->getClient()->getVatApply() ? "cs" : "en"
            )
        );
        // feed generator with xml input using xml exporter
        ExporterXML(gXML->getInput(),true).doExport(i);
        // return id of generated PDF file
        TID fileXML = gXML->closeInput();
        // save generated files with invoice
        InvoiceImpl *ii = dynamic_cast<InvoiceImpl *>(i);
        if (ii == NULL) throw std::bad_cast();
        ii->setFile(filePDF,fileXML);
      }
      catch (...) {
        LOGGER(PACKAGE).error("Exception in ExporterArchiver::doExport.");
        throw;
      }
    }
  };
  
  COMPARE_CLASS_IMPL(InvoiceImpl, CrTime)
  COMPARE_CLASS_IMPL(InvoiceImpl, Number)
  COMPARE_CLASS_IMPL(InvoiceImpl, Registrar)
  COMPARE_CLASS_IMPL(InvoiceImpl, Total)
  COMPARE_CLASS_IMPL(InvoiceImpl, Credit)
  COMPARE_CLASS_IMPL(InvoiceImpl, Type)
  COMPARE_CLASS_IMPL(InvoiceImpl, ZoneName)
  COMPARE_CLASS_IMPL(InvoiceImpl, Price)
  
  
  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //   InvoiceListImpl
  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  /// implementation of interface InvoiceList

  class ListImpl : public Fred::CommonListImpl,
  virtual public List {
    TID idFilter; ///< filter for invoice id 
    TID registrarFilter; ///< filter for registrar recieving invoice
    std::string registrarHandleFilter; ///< filter for registrar by handle 
    TID zoneFilter; ///< filter for id of associated zone
    unsigned typeFilter; ///< filter for invoice type (advance or normal)
    std::string varSymbolFilter; ///< filter for variable symbol of payment
    std::string numberFilter; ///< filter for invoice number
    time_period crDateFilter; ///< filter for crDate
    time_period taxDateFilter; ///< filter for taxDate
    ArchiveFilter archiveFilter; ///< filter for existance of archived file
    TID objectIdFilter; ///< filter for attached object by id
    std::string objectNameFilter; ///< filter for attached object by name
    std::string advanceNumberFilter; ///< filter for source advance invoice
    bool partialLoad; ///< reloads will ignore actions
    ManagerImpl *man; ///< backlink to manager for VAT and others

  public:
    ListImpl(ManagerImpl *_man) : CommonListImpl(),
    idFilter(0), registrarFilter(0), zoneFilter(0), typeFilter(0),
    crDateFilter(ptime(neg_infin),ptime(pos_infin)),
    taxDateFilter(ptime(neg_infin),ptime(pos_infin)),
    archiveFilter(AF_IGNORE), objectIdFilter(0), partialLoad(false),
    man(_man)
    {}

    ~ListImpl() {
      clearList();
    }

    virtual const char* getTempTableName() const {
      return "tmp_invoice_filter_result";
    }
    
    virtual void sort(MemberType _member, bool _asc) {
      switch (_member) {
        case MT_CRDATE:
          stable_sort(data_.begin(), data_.end(), CompareCrTime(_asc));
          break;
        case MT_NUMBER:
          stable_sort(data_.begin(), data_.end(), CompareNumber(_asc));
          break;
        case MT_REGISTRAR:
          stable_sort(data_.begin(), data_.end(), CompareRegistrar(_asc));
          break;
        case MT_TOTAL:
          stable_sort(data_.begin(), data_.end(), CompareTotal(_asc));
          break;
        case MT_CREDIT:
          stable_sort(data_.begin(), data_.end(), CompareCredit(_asc));
          break;
        case MT_TYPE:
          stable_sort(data_.begin(), data_.end(), CompareType(_asc));
          break;
        case MT_ZONE:
          stable_sort(data_.begin(), data_.end(), CompareZoneName(_asc));
          break;
        case MT_PRICE:
          stable_sort(data_.begin(), data_.end(), ComparePrice(_asc));
          break;
        case MT_CRTIME:
          stable_sort(data_.begin(), data_.end(), CompareCrTime(_asc));
          break;
      }
    }

    void clearList() {
      for (unsigned i=0; i<data_.size(); i++)
      delete data_[i];
      data_.clear();
    }
    
    virtual void clearFilter() {
      idFilter = 0;
      registrarFilter = 0;
      registrarHandleFilter = "";
      zoneFilter = 0;
      typeFilter = 0;
      varSymbolFilter = "";
      numberFilter = "";
      crDateFilter = time_period(ptime(neg_infin),ptime(pos_infin));
      taxDateFilter = time_period(ptime(neg_infin),ptime(pos_infin));
      archiveFilter = AF_IGNORE;
      objectIdFilter = 0;
      partialLoad = false;
    }
    
    virtual void setIdFilter(TID id) {
      idFilter = id;
    }
    
    virtual void setRegistrarFilter(TID registrar) {
      registrarFilter = registrar;
    }
    
    virtual void setRegistrarHandleFilter(const std::string& handle) {
      registrarHandleFilter = handle;
    }
    
    virtual void setZoneFilter(TID zone) {
      zoneFilter = zone;
    }
    
    virtual void setTypeFilter(unsigned type) {
      typeFilter = type;
    }
    
    virtual void setVarSymbolFilter(const std::string& varSymbol) {
      varSymbolFilter = varSymbol;
    }
    
    virtual void setNumberFilter(const std::string& number) {
      numberFilter = number;
    }
    
    virtual void setCrDateFilter(const time_period& crDatePeriod) {
      crDateFilter = crDatePeriod;
    }
    
    virtual void setTaxDateFilter(const time_period& taxDatePeriod) {
      taxDateFilter = taxDatePeriod;
    }
    
    virtual void setArchivedFilter(ArchiveFilter archive) {
      archiveFilter = archive;
    }
    
    virtual void setObjectIdFilter(TID objectId) {
      objectIdFilter = objectId;
    }
    
    virtual void setObjectNameFilter(const std::string& objectName) {
      objectNameFilter = objectName;
    }
    
    virtual void setAdvanceNumberFilter(const std::string& number) {
      advanceNumberFilter = number;
    }
    
    virtual void setPartialLoad(bool _partialLoad) {
      partialLoad = _partialLoad;
    }
    
    virtual void reload(Database::Filters::Union& _uf) {
      TRACE("[CALL] Invoicing::ListImpl::reload()");
      clear();
      _uf.clearQueries();

      bool at_least_one = false;
      Database::SelectQuery id_query;
      std::auto_ptr<Database::Filters::Iterator> fit(_uf.createIterator());
      for (fit->first(); !fit->isDone(); fit->next()) {
        Database::Filters::Invoice *inv_filter =
            dynamic_cast<Database::Filters::Invoice*>(fit->get());
        if (!inv_filter)
          continue;

        Database::SelectQuery *tmp = new Database::SelectQuery();
        tmp->addSelect(new Database::Column("id", inv_filter->joinInvoiceTable(), "DISTINCT"));
        _uf.addQuery(tmp);
        at_least_one = true;
      }
      if (!at_least_one) {
        LOGGER(PACKAGE).error("wrong filter passed for reload!");
        return;
      }

      id_query.order_by() << "id DESC";
      id_query.limit(load_limit_);
      _uf.serialize(id_query);

      Database::InsertQuery tmp_table_query = Database::InsertQuery(getTempTableName(),
          id_query);
      LOGGER(PACKAGE).debug(boost::format("temporary table '%1%' generated sql = %2%")
          % getTempTableName() % tmp_table_query.str());

      Database::SelectQuery object_info_query;
      object_info_query.select() << "t_1.id, t_1.zone, t_2.fqdn, "
                                 << "t_1.crdate::timestamptz AT TIME ZONE 'Europe/Prague', "
                                 << "t_1.taxdate, t_5.fromdate, t_5.todate, t_4.typ, t_1.prefix, "
                                 << "t_1.registrarid, t_1.credit, t_1.price, "
                                 << "t_1.vat, t_1.total, t_1.totalvat, "
                                 << "t_1.file, t_1.fileXML, t_3.organization, t_3.street1, "
                                 << "t_3.city, t_3.postalcode, "
                                 << "TRIM(t_3.ico), TRIM(t_3.dic), TRIM(t_3.varsymb), "
                                 << "t_3.handle, t_3.vat, t_3.id, t_3.country, "
                                 << "t_6.name as file_name, t_7.name as filexml_name";

      object_info_query.from() << "tmp_invoice_filter_result tmp "
                               << "JOIN invoice t_1 ON (tmp.id = t_1.id) "
                               << "JOIN zone t_2 ON (t_1.zone = t_2.id) "
                               << "JOIN registrar t_3 ON (t_1.registrarid = t_3.id) "
                               << "JOIN invoice_prefix t_4 ON (t_4.id = t_1.prefix_type) "
                               << "LEFT JOIN invoice_generation t_5 ON (t_1.id = t_5.invoiceid) "
                               << "LEFT JOIN files t_6 ON (t_1.file = t_6.id) "
                               << "LEFT JOIN files t_7 ON (t_1.filexml = t_7.id)";

      object_info_query.order_by() << "tmp.id";

      try {
        Database::Connection conn = Database::Manager::acquire();

        Database::Query create_tmp_table("SELECT create_tmp_table('" + std::string(getTempTableName()) + "')");
        conn.exec(create_tmp_table);
        conn.exec(tmp_table_query);


        Database::Result r_info = conn.exec(object_info_query);
        for (Database::Result::Iterator it = r_info.begin(); it != r_info.end(); ++it) {
          Database::Row::Iterator col = (*it).begin();

          Database::ID       id             = *col;
          Database::ID       zone           = *(++col);
          std::string        fqdn           = *(++col);
          Database::DateTime create_time    = *(++col);
          Database::Date     tax_date       = *(++col);
          Database::Date     from_date      = *(++col);
          Database::Date     to_date        = *(++col);
          Type               type           = (int)*(++col) == 0 ? IT_DEPOSIT 
                                                                 : IT_ACCOUNT;
          unsigned long long number         = *(++col);
          Database::ID       registrar_id   = *(++col);
          Database::Money    credit         = *(++col);
          Database::Money    price          = *(++col);
          short              vat_rate       = *(++col);
          Database::Money    total          = *(++col);
          Database::Money    total_vat      = *(++col);
          Database::ID       filePDF        = *(++col);
          Database::ID       fileXML        = *(++col);
          std::string        c_organization = *(++col);
          std::string        c_street1      = *(++col);
          std::string        c_city         = *(++col);
          std::string        c_postal_code  = *(++col);
          std::string        c_ico          = *(++col);
          std::string        c_dic          = *(++col);
          std::string        c_var_symb     = *(++col);
          std::string        c_handle       = *(++col);
          bool               c_vat          = *(++col);
          TID                c_id           = *(++col);
          std::string        c_country      = *(++col);
          std::string        filepdf_name   = *(++col);
          std::string        filexml_name   = *(++col);

          date_period account_period(from_date, to_date);
          SubjectImpl client(c_id, c_handle, c_organization, "", c_street1,
              c_city, c_postal_code, c_country, c_ico, c_dic,
              "", "", "", "", "", "", c_vat);
                                                                     
          data_.push_back(new InvoiceImpl(id,
                                          zone,
                                          fqdn,
                                          create_time,
                                          tax_date,
                                          account_period,
                                          type,
                                          number,
                                          registrar_id,
                                          credit,
                                          price,
                                          vat_rate,
                                          total,
                                          total_vat,
                                          filePDF,
                                          fileXML,
                                          c_var_symb,
                                          client,
                                          filepdf_name,
                                          filexml_name,
                                          man));
          
        }
        
        LOGGER(PACKAGE).debug(boost::format("list of invoices size: %1%") % data_.size());
        
        if (data_.empty())
          return;
        
        /*
         * load details to each invoice...
         * 
         * 
         * append list of sources to all selected invoices
         */
        resetIDSequence();
        Database::SelectQuery source_query;
        source_query.select() << "tmp.id, ipm.credit, sri.vat, sri.prefix, "
                              << "ipm.balance, sri.id, sri.total, "
                              << "sri.totalvat, sri.crdate";
        source_query.from() << "tmp_invoice_filter_result tmp "
                            << "JOIN invoice_credit_payment_map ipm ON (tmp.id = ipm.invoiceid) "
                            << "JOIN invoice sri ON (ipm.ainvoiceid = sri.id) ";
        source_query.order_by() << "tmp.id";
        
        resetIDSequence();
        Database::Result r_sources = conn.exec(source_query);
        for (Database::Result::Iterator it = r_sources.begin(); it != r_sources.end(); ++it) {
          Database::Row::Iterator col = (*it).begin();
          Database::ID invoice_id = *col;
                    
          InvoiceImpl *invoice_ptr = dynamic_cast<InvoiceImpl*>(findIDSequence(invoice_id));
          if (invoice_ptr) 
            invoice_ptr->addSource(++col);
        }
                  
        /* append list of actions to all selected invoices
         * it handle situation when action come from source advance invoices
         * with different vat rates by grouping
         * this is ignored on partial load
         */
        if (!partialLoad) {
          Database::SelectQuery action_query;
          action_query.select() << "tmp.id, SUM(ipm.price), i.vat, o.name, "
                                << "ior.crdate::timestamptz AT TIME ZONE 'Europe/Prague', "
                                << "ior.exdate, ior.operation, ior.period, "
                                << "CASE "
                                << "  WHEN ior.period = 0 THEN 0 "
                                << "  ELSE SUM(ipm.price) * 12 / ior.period END, "
                                << "o.id";
          action_query.from() << "tmp_invoice_filter_result tmp "
                              << "JOIN invoice_object_registry ior ON (tmp.id = ior.invoiceid) "
                              << "JOIN object_registry o ON (ior.objectid = o.id) "
                              << "JOIN invoice_object_registry_price_map ipm ON (ior.id = ipm.id) "
                              << "JOIN invoice i ON (ipm.invoiceid = i.id) ";
          action_query.group_by() << "tmp.id, o.name, ior.crdate, ior.exdate, "
                                  << "ior.operation, ior.period, o.id, i.vat";
          action_query.order_by() << "tmp.id";
        
          resetIDSequence();
          Database::Result r_actions = conn.exec(action_query);
          for (Database::Result::Iterator it = r_actions.begin(); it != r_actions.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();
            Database::ID invoice_id = *col;
            
            InvoiceImpl *invoice_ptr = dynamic_cast<InvoiceImpl* >(findIDSequence(invoice_id));
            if (invoice_ptr) 
              invoice_ptr->addAction(++col);
          }
        }
        /* checks if row number result load limit is active and set flag */ 
        CommonListImpl::reload();
      }
      catch (Database::Exception& ex) {
            std::string message = ex.what();
            if (message.find(Database::Connection::getTimeoutString())
                    != std::string::npos) {
                LOGGER(PACKAGE).info("Statement timeout in request list.");
                clear();
                throw;
            } else {
                LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
                clear();
            }
        }
      catch (std::exception& ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        clear();
      }

    }    
    virtual void reload()
    {
        Logging::Context ctx("invoice list reload()");
        try
        {
            Database::Connection conn = Database::Manager::acquire();
            clearList();

            Database::QueryParams sql_params;//query params
            sql_params.reserve(20);
            std::stringstream sql;
            // id that conform to filter will be stored in temporary table
            // sql is contructed from two sections 'from' and 'where'
            // that are pasted into final 'sql' stream
            sql << "SELECT DISTINCT i.id "
            << "INTO TEMPORARY tmp_invoice_filter_result ";
            std::stringstream from;
            from << "FROM invoice i ";
            std::stringstream where;
            where << "WHERE 1=1 "; // must be for the case of empty filter
            // process individual filters
            if (idFilter)
            {
                sql_params.push_back(boost::lexical_cast<std::string>(idFilter));
                where << "AND " << "i.id" << "= $" << sql_params.size() << "::bigint ";
            }
            if (registrarFilter)
            {
                sql_params.push_back(boost::lexical_cast<std::string>(registrarFilter));
                where << "AND " << "i.registrarid" << "= $" << sql_params.size() << "::bigint ";
            }
            if (zoneFilter)
            {
                sql_params.push_back(boost::lexical_cast<std::string>(zoneFilter));
                where << "AND " << "i.zone" << "= $" << sql_params.size() << "::bigint ";
            }
            if (typeFilter && typeFilter<=2)
            {
                from << ", invoice_prefix ip ";
                where << "AND i.prefix_type=ip.id ";
                sql_params.push_back(boost::lexical_cast<std::string>(typeFilter-1));
                where << "AND " << "ip.typ" << "=$" << sql_params.size() << "::integer ";
            }
            if (!varSymbolFilter.empty() || !registrarHandleFilter.empty())
            {
                from << ", registrar r ";
                where << "AND i.registrarid=r.id ";
                if (!varSymbolFilter.empty())
                {
                    sql_params.push_back(boost::lexical_cast<std::string>(varSymbolFilter));
                    where << "AND "
                           << "TRIM(r.varsymb)" << " ILIKE TRANSLATE($" << sql_params.size() << "::text,'*?','%_') ";
                }
                if (!registrarHandleFilter.empty())
                {
                    sql_params.push_back(boost::lexical_cast<std::string>(registrarHandleFilter));
                    where << "AND "
                           << "r.handle" << " ILIKE TRANSLATE($" << sql_params.size() << "::text,'*?','%_') ";
                }
            }
            if (!numberFilter.empty())
            {
                sql_params.push_back(boost::lexical_cast<std::string>(numberFilter));
                where << "AND "
                    << "i.prefix" << " ILIKE TRANSLATE($" << sql_params.size() << "::text,'*?','%_') ";
            }
            if (!crDateFilter.begin().is_special())
            {
                sql_params.push_back(boost::lexical_cast<std::string>(
                        to_iso_extended_string(crDateFilter.begin().date())));
                 where << "AND " << "i.crdate" << ">=$"
                   <<  sql_params.size() << "::text ";
            }
            if (!crDateFilter.end().is_special())
            {
                sql_params.push_back(boost::lexical_cast<std::string>(
                        to_iso_extended_string(crDateFilter.end().date())
                        )+ " 23:59:59");
                where << "AND " << "i.crdate" << "<=$"
                        <<  sql_params.size() << "::text ";
            }
            if ((!taxDateFilter.begin().is_special()))
            {
                sql_params.push_back(boost::lexical_cast<std::string>(
                        to_iso_extended_string(taxDateFilter.begin())));
               where << "AND " << "i.taxdate" << ">=$"
                 <<  sql_params.size() << "::text ";
            }
            if ((!taxDateFilter.end().is_special()))
            {
                sql_params.push_back(boost::lexical_cast<std::string>(
                        to_iso_extended_string(taxDateFilter.end())));
                where << "AND " << "i.taxdate" << "<=$"
                    << sql_params.size() << "::text ";
            }
            switch (archiveFilter)
            {
                case AF_IGNORE: break;
                case AF_SET: where << "AND NOT(i.file ISNULL) "; break;
                case AF_UNSET: where << "AND i.file ISNULL "; break;
                default: break;
            }
            if (objectIdFilter)
            {
                from << ", invoice_object_registry ior ";
                where << "AND i.id=ior.invoiceid ";
                sql_params.push_back(boost::lexical_cast<std::string>(objectIdFilter));
                where << "AND " << "ior.objectid" << "=$" << sql_params.size() << "::bigint ";
            }
            if (!objectNameFilter.empty())
            {
                from << ", invoice_object_registry iorh, object_registry obr ";
                where << "AND i.id=iorh.invoiceid AND obr.id=iorh.objectid ";
                sql_params.push_back(boost::lexical_cast<std::string>(objectNameFilter));
                where << "AND "
                       << "obr.name" << " ILIKE TRANSLATE($" << sql_params.size() << "::text,'*?','%_') ";
            }

            if (!advanceNumberFilter.empty())
            {
                from << ", invoice_object_registry ior2 "
                << ", invoice_object_registry_price_map iorpm "
                << ", invoice advi ";
                where << "AND i.id=ior2.invoiceid "
                << "AND iorpm.id=ior2.id AND iorpm.invoiceid=advi.id ";
                sql_params.push_back(boost::lexical_cast<std::string>(advanceNumberFilter));
                where << "AND "
                       << "advi.prefix" << " ILIKE TRANSLATE($" << sql_params.size() << "::text,'*?','%_') ";
            }
            // complete sql end do the query
            sql << from.rdbuf() << where.rdbuf();

            Database::Result res1 = conn.exec_params(sql.str(), sql_params);
            Database::Result res2 = conn.exec("ANALYZE tmp_invoice_filter_result");
            // initialize list of invoices using temporary table

            Database::Result res3 = conn.exec(
                  "SELECT "
                  " i.id, i.zone, i.crdate::timestamptz AT TIME ZONE 'Europe/Prague',"
                  " i.taxdate, ig.fromdate, "
                  " ig.todate, ip.typ, i.prefix, i.registrarid, i.credit, "
                  " i.price, i.vat, i.total, i.totalvat, "
                  " i.file, i.fileXML, "
                  " r.organization, r.street1, "
                  " r.city, r.postalcode, TRIM(r.ico), TRIM(r.dic), TRIM(r.varsymb), "
                  " r.handle, r.vat, r.id, z.fqdn, r.country "
                  "FROM "
                  " tmp_invoice_filter_result it "
                  " JOIN invoice i ON (it.id=i.id) "
                  " JOIN zone z ON (i.zone=z.id) "
                  " JOIN registrar r ON (i.registrarid=r.id) "
                  " JOIN invoice_prefix ip ON (ip.id=i.prefix_type) "
                  " LEFT JOIN invoice_generation ig ON (i.id=ig.invoiceid) "
                  // temporary static sorting
                  " ORDER BY it.id"
              );

            for (unsigned i=0; i < res3.size(); ++i)
            {
                Database::ID       id             = res3[i][0];
                Database::ID       zone           = res3[i][1];
                std::string        fqdn           = res3[i][26];
                Database::DateTime create_time    = (ptime(res3[i][2].isnull()
                                                        ? ptime(not_a_date_time)
                                                        : time_from_string(res3[i][2])));
                Database::Date     tax_date       = (date(res3[i][3].isnull()
                                                        ? date(not_a_date_time)
                                                        : from_string(res3[i][3])));
                Database::Date     from_date      = (date(res3[i][4].isnull()
                                                        ? date(neg_infin)
                                                        : from_string(res3[i][4])));
                Database::Date     to_date        = (date(res3[i][5].isnull()
                                                        ? date(pos_infin)
                                                        : from_string(res3[i][5])));
                Type               type           = (int(res3[i][6]) == 0 ? IT_DEPOSIT
                                                                       : IT_ACCOUNT);
                unsigned long long number         = res3[i][7];
                Database::ID       registrar_id   = res3[i][8];

                if (get_price(std::string(res3[i][9])) > 2147483647UL)
                {
                    LOGGER(PACKAGE).error(boost::format(
                                    "reload() credit  %1% > 2147483647UL")
                                    % std::string(res3[i][9]));
                    continue;
                }
                Database::Money    credit         = res3[i][9];

                if (get_price(std::string(res3[i][10])) > 2147483647UL)
                {
                    LOGGER(PACKAGE).error(boost::format(
                                    "reload() price  %1% > 2147483647UL")
                                    % std::string(res3[i][10]));
                    continue;
                }
                Database::Money    price          = res3[i][10];

                short              vat_rate       = res3[i][11];

                if (get_price(std::string(res3[i][12])) > 2147483647UL)
                {
                    LOGGER(PACKAGE).error(boost::format(
                                    "reload() total  %1% > 2147483647UL")
                                    % std::string(res3[i][12]));
                    continue;
                }
                Database::Money    total          = res3[i][12];

                if (get_price(std::string(res3[i][13])) > 2147483647UL)
                {
                    LOGGER(PACKAGE).error(boost::format(
                                    "reload() total_vat  %1% > 2147483647UL")
                                    % std::string(res3[i][13]));
                    continue;
                }
                Database::Money    total_vat      = res3[i][13];

                Database::ID       filePDF        = res3[i][14];
                Database::ID       fileXML        = res3[i][15];

                std::string        c_organization = "";
                std::string        c_street1      = res3[i][17];
                std::string        c_city         = res3[i][18];
                std::string        c_postal_code  = res3[i][19];
                std::string        c_ico          = res3[i][20];
                std::string        c_dic          = res3[i][21];
                std::string        c_var_symb     = res3[i][22];
                std::string        c_handle       = res3[i][23];
                bool               c_vat          = res3[i][24];
                TID                c_id           = res3[i][25];
                std::string        c_country      = res3[i][27];
                std::string        filepdf_name   = "";
                std::string        filexml_name   = "";

                date_period account_period(from_date, to_date);
                SubjectImpl client(c_id, c_handle, c_organization, "", c_street1,
                    c_city, c_postal_code, c_country, c_ico, c_dic,
                    "", "", "", "", "", "", c_vat);

                data_.push_back(new InvoiceImpl(id,
                                                zone,
                                                fqdn,
                                                create_time,
                                                tax_date,
                                                account_period,
                                                type,
                                                number,
                                                registrar_id,
                                                credit,
                                                price,
                                                vat_rate,
                                                total,
                                                total_vat,
                                                filePDF,
                                                fileXML,
                                                c_var_symb,
                                                client,
                                                filepdf_name,
                                                filexml_name,
                                                man));
            }//for res3

            /* append list of actions to all selected invoices
             * it handle situation when action come from source advance invoices
             * with different vat rates by grouping
             * this is ignored on partial load
             */
            if (!partialLoad) {
              Database::SelectQuery action_query;
              action_query.select() << "tmp.id, SUM(ipm.price), i.vat, o.name, "
                                    << "ior.crdate::timestamptz AT TIME ZONE 'Europe/Prague', "
                                    << "ior.exdate, ior.operation, ior.period, "
                                    << "CASE "
                                    << "  WHEN ior.period = 0 THEN 0 "
                                    << "  ELSE SUM(ipm.price) * 12 / ior.period END, "
                                    << "o.id";
              action_query.from() << "tmp_invoice_filter_result tmp "
                                  << "JOIN invoice_object_registry ior ON (tmp.id = ior.invoiceid) "
                                  << "JOIN object_registry o ON (ior.objectid = o.id) "
                                  << "JOIN invoice_object_registry_price_map ipm ON (ior.id = ipm.id) "
                                  << "JOIN invoice i ON (ipm.invoiceid = i.id) ";
              action_query.group_by() << "tmp.id, o.name, ior.crdate, ior.exdate, "
                                      << "ior.operation, ior.period, o.id, i.vat";
              action_query.order_by() << "tmp.id";

              resetIDSequence();
              Database::Result r_actions = conn.exec(action_query);
              for (Database::Result::Iterator it = r_actions.begin(); it != r_actions.end(); ++it) {
                Database::Row::Iterator col = (*it).begin();
                Database::ID invoice_id = *col;

                InvoiceImpl *invoice_ptr = dynamic_cast<InvoiceImpl* >(findIDSequence(invoice_id));
                if (invoice_ptr)
                  invoice_ptr->addAction(++col);
              }
            }

            //append list of sources to all selected invoices
            {
                resetIDSequence();
                Database::SelectQuery source_query;
                source_query.select() << "tmp.id, ipm.credit, sri.vat, sri.prefix, "
                                      << "ipm.balance, sri.id, sri.total, "
                                      << "sri.totalvat, sri.crdate";
                source_query.from() << "tmp_invoice_filter_result tmp "
                                    << "JOIN invoice_credit_payment_map ipm ON (tmp.id = ipm.invoiceid) "
                                    << "JOIN invoice sri ON (ipm.ainvoiceid = sri.id) ";
                source_query.order_by() << "tmp.id";

                resetIDSequence();
                Database::Result r_sources = conn.exec(source_query);
                for (Database::Result::Iterator it = r_sources.begin(); it != r_sources.end(); ++it) {
                  Database::Row::Iterator col = (*it).begin();
                  Database::ID invoice_id = *col;

                  InvoiceImpl *invoice_ptr = dynamic_cast<InvoiceImpl*>(findIDSequence(invoice_id));
                  if (invoice_ptr)
                    invoice_ptr->addSource(++col);
                }
            }
            // delete temporary table
            conn.exec("DROP TABLE tmp_invoice_filter_result ");

        }//try
        catch (Database::Exception& ex)
        {
            std::string message = ex.what();
            if (message.find(Database::Connection::getTimeoutString())
                  != std::string::npos)
            {
              LOGGER(PACKAGE).error("timeout");
              clear();
            }
            else
            {
              LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
              clear();
            }
            throw;
        }
        catch (std::exception& ex)
        {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            clear();
            throw;
        }

    }//reload()

    
    /// export all invoices on the list using given exporter
    InvoiceIdVect doExport(Exporter *_exporter) {
        if(!_exporter) {
            LOGGER(PACKAGE).error("Exporter::doExport _exporter");
            throw std::runtime_error("Exporter::doExport _exporter");
        }
      InvoiceIdVect ret_inv_id;
      for (Iterator it = data_.begin(); it != data_.end(); ++it) {
        InvoiceImpl *invoice = dynamic_cast<InvoiceImpl*>(*it);
        if (invoice)
        {
          ret_inv_id.push_back(invoice->getId());
          invoice->doExport(_exporter);
        }
        else
        {
            LOGGER(PACKAGE).error("Exporter::doExport dynamic_cast<InvoiceImpl*> failed");
            throw std::runtime_error("Exporter::doExport dynamic_cast<InvoiceImpl*> failed");
        }
      }
      return ret_inv_id;
    }

    virtual Invoice* get(unsigned _idx) const {
      try {
        Invoice *request = dynamic_cast<Invoice*>(data_.at(_idx));
        if (request)
        return request;
        else
        throw std::exception();
      }
      catch (...) {
        throw std::exception();
      }
    }

    virtual void exportXML(std::ostream& out) {
      out << "<?xml version='1.0' encoding='utf-8'?>";
      ExporterXML xml(out,false);
      if (getCount()!=1) out << TAGSTART(list);
      doExport(&xml);
      if (getCount()!=1) out << TAGEND(list);
    }
    
    /// dummy implementation of method from CommonObjet
    virtual void makeQuery(bool, bool, std::stringstream&) const {
      
    }


  
  virtual bool isEmpty() { return (size() == 0); }

  virtual unsigned int getSize() const 
  {
        return size();
  }
    
  }; // ListImpl 
  
  
  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //   Mails
  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  /// send mails with invoices to registrars 
  class Mails {
    /// describe one email notification about invoice or invoice generation
    struct Item {
      std::string registrarEmail; ///< address to deliver email
      date from; ///< start of invoicing period 
      date to; ///< end of invoicing period 
      TID filePDF; ///< id of pdf file with invoice attached in email
      TID fileXML; ///< id of xml file with invoice attached in email
      TID generation; ///< filled if source is invoice generation
      TID invoice; ///< filled if successful generation or advance invoice
      TID mail; ///< id of generated email
      std::string zoneFqdn;
      
      const char *getTemplateName() {
        if (!generation) return "invoice_deposit";
        if (!invoice) return "invoice_noaudit";
        return "invoice_audit";
      }
      
      Item(const std::string& _registrarEmail, date _from, date _to,
           TID _filePDF, TID _fileXML, TID _generation, TID _invoice, TID _mail,
           const std::string& _zoneFqdn) :
               registrarEmail(_registrarEmail), 
               from(_from), 
               to(_to),
               filePDF(_filePDF), 
               fileXML(_fileXML),
               generation(_generation), 
               invoice(_invoice), 
               mail(_mail),
               zoneFqdn(_zoneFqdn) {
      }
    };
    
    typedef std::vector<Item> MailItems; ///< type for notification list
    MailItems items; ///< list of notifications to send
    Mailer::Manager *mm; ///< mail sending interface
    /// store information about sending email 
    void store(unsigned idx) {
       
      std::stringstream sql;
      sql << "INSERT INTO invoice_mails (invoiceid,genid,mailid) VALUES (";
      if (items[idx].invoice) sql << items[idx].invoice;
      else sql << "NULL";
      sql << ",";
      if (items[idx].generation) sql << items[idx].generation;
      else sql << "NULL";
      sql << ","
      << items[idx].mail
      << ")";

      Database::Connection conn = Database::Manager::acquire();
      conn.exec(sql.str());
    }
    
  public:
    Mails(Mailer::Manager *_mm) : mm(_mm) {
    }
    /// send all mails and store information about sending
    void send() { //throw (SQL_ERROR) {
      for (unsigned i=0; i<items.size(); i++) {

          try {

        Item *it = &items[i];
        Mailer::Parameters params;
        std::stringstream dateBuffer;
        dateBuffer.imbue(std::locale(
                dateBuffer.getloc(),
                new date_facet("%d.%m.%Y")
            ));
        dateBuffer << it->from;
        params["fromdate"] = dateBuffer.str();
        dateBuffer.str("");
        dateBuffer << it->to;
        params["todate"] = dateBuffer.str();
        params["zone"] = it->zoneFqdn;
        Mailer::Handles handles;
        // TODO: include domain or registrar handles??
        Mailer::Attachments attach;
        if (it->filePDF) attach.push_back(it->filePDF);
        if (it->fileXML) attach.push_back(it->fileXML);
        it->mail = mm->sendEmail(
            "", // default sender according to template 
            it->registrarEmail,
            "", // default subject according to template
            it->getTemplateName(),
            params, handles, attach
        );
        if (!it->mail) {
          // TODO: LOG ERROR 
            LOGGER(PACKAGE).error(
                    std::string(" Error while send mail in Mails class email: ")
                    + it->registrarEmail
                    + " pdf file id: " + boost::lexical_cast<std::string>(it->filePDF)
                    + " xml file id: " + boost::lexical_cast<std::string>(it->fileXML)
                    + " invoice id: " + boost::lexical_cast<std::string>(it->invoice)
                    + " zone fqdn: " + it->zoneFqdn
            );
            continue;
        }
        store(i);
          } catch (std::exception ex)
          {
              LOGGER(PACKAGE).error(
                      std::string(" std exception while send mail in Mails class what: ")
                      + ex.what()
              );
              continue;
          }
      }//for i
    }
 
    void load()
    {
        Database::Connection conn = Database::Manager::acquire();

        std::stringstream sql;

        sql << "SELECT r.email, g.fromdate, g.todate, "
        << "i.file, i.fileXML, g.id, i.id, z.fqdn "
        << "FROM registrar r, invoice i "
        << "LEFT JOIN invoice_generation g ON (g.invoiceid=i.id) "
        << "LEFT JOIN invoice_mails im ON (im.invoiceid=i.id) "
        << "LEFT JOIN zone z ON (z.id = i.zone) "
        << "WHERE i.registrarid=r.id "
        << "AND im.mailid ISNULL "
        << "AND NOT(r.email ISNULL OR TRIM(r.email)='')"
        << "UNION "
        << "SELECT r.email, g.fromdate, g.todate, NULL, NULL, g.id, "
        << "NULL, z.fqdn "
        << "FROM registrar r, invoice_generation g "
        << "LEFT JOIN invoice_mails im ON (im.genid=g.id) "
        << "LEFT JOIN zone z ON (z.id = g.zone) "
        << "WHERE g.registrarid=r.id AND g.invoiceid ISNULL "
        << "AND im.mailid ISNULL "
        << "AND NOT(r.email ISNULL OR TRIM(r.email)='')";

        Database::Result res = conn.exec(sql.str());
        for (unsigned i=0; i < res.size(); ++i)
        items.push_back(Item(
              res[i][0],
              (date(res[i][1].isnull()? date(not_a_date_time) : from_string(res[i][1]))),
              (date(res[i][2].isnull()? date(not_a_date_time) : from_string(res[i][2]))),
              res[i][3],
              res[i][4],
              res[i][5],
              res[i][6],
              (TID)0,
              res[i][7]
          ));
    }//load
  }; // Mails
  

  void ManagerImpl::initVATList()  {

    if (vatList.empty())
    {
      Database::Connection conn = Database::Manager::acquire();
      Database::Result res = conn.exec("SELECT vat, 10000*koef, valid_to FROM price_vat");
      for (unsigned i=0; i < res.size(); ++i) {
        vatList.push_back(
            VAT(res[i][0],
                res[i][1],
                (date(res[i][2].isnull()? date(not_a_date_time) : from_string(res[i][2])))
                )
        );
      }//for res
    }//if empty
  }


  Money ManagerImpl::countVAT(Money price, unsigned vatRate, bool base) {
    const VAT *v = getVAT(vatRate);
    unsigned coef = v ? v->koef : 0;
    return price * coef / (10000 - (base ? coef : 0));
  }
  
  const VAT * ManagerImpl::getVAT(unsigned rate) {
    // late initialization would brake constness
    this->initVATList();
    std::vector<VAT>::const_iterator ci = find(
        vatList.begin(),vatList.end(),rate
    );
    return ci == vatList.end() ? NULL : &(*ci);
  }
  
  InvoiceIdVect ManagerImpl::archiveInvoices(bool send) {
      
      if(docman == NULL || mailman == NULL) {
        LOGGER(PACKAGE).error("archiveInvoices: No docman or mailman specified in c-tor. ");
        throw std::runtime_error("archiveInvoices: No docman or mailman specified in c-tor");
      }
      
      InvoiceIdVect ret_inv;

    try {
      // archive unarchived invoices
      ExporterArchiver exporter(docman);
      ListImpl l(this);
      l.setArchivedFilter(ListImpl::AF_UNSET);
      l.reload();
      ret_inv = l.doExport(&exporter);
      if (send) {
        Mails m(mailman);
        m.load();
        m.send();
      }
    }
    catch (...) {
      LOGGER(PACKAGE).error("Exception in archiveInvoices.");
      throw;
    }
    return ret_inv;
  }
  
  List* ManagerImpl::createList() {
    return new ListImpl(this);
    // return new ListImpl(conn_, (ManagerImpl *)this);
  }
  
  std::string ManagerImpl::getCreditByZone(const std::string& registrarHandle, TID zone) {
      Database::Connection conn = Database::Manager::acquire();
    std::string sql
    ("SELECT (COALESCE(SUM(credit), 0))::numeric(1000,2) "
     "FROM invoice i JOIN registrar r ON (i.registrarid=r.id) "
     "WHERE i.zone = $1::bigint AND r.handle = $2::text");

    Database::Result res = conn.exec_params(sql
            , Database::query_param_list(zone)(registrarHandle));
    std::string result = std::string(res[0][0]);

    LOGGER(PACKAGE).debug(std::string("ManagerImpl::getCreditByZone res[0][0]: ")
        + result
        + " registrarHandle: " + registrarHandle
        + " zone id: " + boost::lexical_cast<std::string>(zone)
        + " sql: " + sql
    );
    return result;
    
  }

  bool ManagerImpl::insertInvoicePrefix(unsigned long long zoneId,
          int type, int year, unsigned long long prefix) 
  {
      TRACE("Invoicing::Manager::insertInvoicePrefix(...)");
      Database::Connection conn = Database::Manager::acquire();
      std::stringstream query;
      query << "INSERT INTO invoice_prefix (zone, typ, year, prefix) VALUES"
          << "(" << zoneId << ", "
          << type << ", "
          << year << ", "
          << prefix << ");";
      try {
          conn.exec(query.str());
      } catch (...) {
          return false;
      }
      return true;
  }

  bool ManagerImpl::insertInvoicePrefix(const std::string &zoneName,
          int type, int year, unsigned long long prefix) 
  {
      TRACE("Invoicing::Manager::insertInvoicePrefix(...)");

      Database::Connection conn = Database::Manager::acquire();
      boost::format query = boost::format("select id from zone where fqdn = %1%")
                                          % Database::Value(zoneName);
      Database::Result res = conn.exec(query.str());

      return insertInvoicePrefix((unsigned long long)res[0][0], type, year, prefix);
  }
  
 Manager *Manager::create(
                           Document::Manager *_doc_manager, 
                           Mailer::Manager *_mail_manager) {
    return new ManagerImpl(_doc_manager, _mail_manager);
  }

 Manager *Manager::create()
 {
     return new ManagerImpl();
 }

  
}; // Invoicing
}; // Fred
