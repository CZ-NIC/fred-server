/*
 * Copyright (C) 2007-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/libfred/common_impl.hh"
#include "src/deprecated/libfred/documents.hh"
#include "src/deprecated/libfred/invoicing/exceptions.hh"
#include "src/deprecated/libfred/invoicing/invoice.hh"
#include "libfred/poll/get_request_fee_message.hh"
#include "util/log/context.hh"
#include "util/log/logger.hh"
#include "util/types/convert_sql_db_types.hh"
#include "util/types/sqlize.hh"

#include <boost/algorithm/string.hpp>
#include <boost/checked_delete.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

using namespace boost::gregorian;
using namespace boost::posix_time;


namespace LibFred {
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

// hold vat rates for time periods
class VAT {
public:
  VAT(Decimal _vatRate, Decimal _koef, date _validity) :
    vatRate(_vatRate), koef(_koef), validity(_validity) {
  }
  bool operator==(Decimal rate) const {
    return vatRate == rate;
  }
  Decimal vatRate; ///< percent rate
  Decimal koef; ///< koeficient for VAT counting
  date validity; ///< valid to this date
};

struct AdvanceInvoice
{
    unsigned long long invoice_id;
    Money balance;
    Money credit_change;
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
  Money countVAT(Money price_without_vat, Decimal vatRate, bool use_coef);

  const VAT *getVAT(Decimal rate);
  /// find unarchived invoices. archive then and send them by email
  InvoiceIdVect archiveInvoices(bool send, InvoiceIdVect archive_only_this_if_set = InvoiceIdVect());
  /// create empty list of invoices      
  virtual List* createList();
  /// return credit for registrar by zone
  virtual std::string
      getCreditByZone(const std::string& registrarHandle, TID zone);
  virtual bool insertInvoicePrefix(unsigned long long zoneId,
          int type, int year, unsigned long long prefix);
  virtual bool insertInvoicePrefix(const std::string &zoneName,
          int type, int year, unsigned long long prefix);

  virtual void createInvoicePrefixes(bool for_current_year);
  virtual void addInvoiceNumberPrefix( unsigned long prefix
            , const std::string& zone_fqdn
            , const std::string invoice_type_name);


  void invoiceLowerCredit(Database::Connection &conn, Money price, Database::ID invoiceID) {

      if(price == Money("0")) {
          LOGGER.debug ( boost::format("Zero price for invoiceID %1%, credit unchanged.") % invoiceID);
          return;
      }
      conn.exec_params("UPDATE invoice SET credit = credit - $1::numeric(10,2) WHERE id=$2::integer",
              Database::query_param_list(price.get_string()) (invoiceID));
  }

  // WARNING: this is called from epp_impl.cc and it's sharing connection with dbsql's DB
  // so it must *NOT* create Database::Transactions
  virtual bool charge_operation_auto_price(
      const std::string& operation
      , unsigned long long zone_id
      , unsigned long long registrar_id
      , unsigned long long object_id
      , boost::posix_time::ptime crdate //utc timestamp
      , boost::gregorian::date date_from //local date included in interval
      , boost::gregorian::date date_to //local date not included in interval, can be unspecified
      , Decimal quantity)
  {
      //if (registrar is "system registrar") return ok //no charging

      // TODO we rely that connection is saved in thread specific data
      Database::Connection conn = Database::Manager::acquire();

      // find out whether the registrar in question is system registrar
      bool system = false;
      Database::Result rsys =
        conn.exec_params("SELECT system FROM registrar WHERE id=$1::integer",
            Database::query_param_list(registrar_id));
      if(rsys.size() != 1 || rsys[0][0].isnull()) {
          throw std::runtime_error((boost::format("Registrar ID %1% not found ") % registrar_id).str());
      } else {
          system = rsys[0][0];
          if(system) {
              LOGGER.info ( (boost::format("Registrar ID %1% has system flag set, not billing") % registrar_id).str());
              // no billing for system registrar
              return true;
          }
      }

      //get_operation_payment_settings
      Database::Result operation_price_list_result
          = conn.exec_params(
          "SELECT enable_postpaid_operation, operation_id, price, quantity"
              " FROM price_list pl "
                  " JOIN enum_operation eo ON pl.operation_id = eo.id "
                  " JOIN zone z ON z.id = pl.zone_id "
              " WHERE pl.valid_from < $1::timestamp "
                    " AND (pl.valid_to is NULL OR pl.valid_to > $1::timestamp ) "
              " AND pl.zone_id = $2::bigint AND eo.operation = $3::text "
              " ORDER BY pl.valid_from DESC "
              " LIMIT 1 "
          , Database::query_param_list(crdate)(zone_id)(operation));

      if(operation_price_list_result.size() != 1)
      {
          throw std::runtime_error("charge_operation_auto_price: operation not found");
      }

      bool enable_postpaid_operation = operation_price_list_result[0][0];
      unsigned long long operation_id = operation_price_list_result[0][1];
      Money price_list_price = std::string(operation_price_list_result[0][2]);
      Decimal  price_list_quantity = std::string(operation_price_list_result[0][3]);

      if(price_list_quantity == Decimal("0"))
      {
          throw std::runtime_error(
                  "charge_operation_auto_price: price_list_quantity == 0");
      }

      Money price =  price_list_price
              * quantity
              / price_list_quantity;//count_price

      //get_registrar_credit - lock record in registrar_credit table for registrar and zone
      Database::Result locked_registrar_credit_result
          = conn.exec_params(
          "SELECT id, credit "
               " FROM registrar_credit "
               " WHERE registrar_id = $1::bigint "
                   " AND zone_id = $2::bigint "
           " FOR UPDATE "
          , Database::query_param_list(registrar_id)(zone_id));

      if(locked_registrar_credit_result.size() != 1)
      {
          std::string errmsg = str( boost::format("ManagerImpl::charge_operation"
                  " zone_id %1% registrar_id %2% unable to get registrar_credit")
          % zone_id % registrar_id );

          LOGGER.error(errmsg);
          throw std::runtime_error(errmsg);
      }

      unsigned long long registrar_credit_id = locked_registrar_credit_result[0][0];
      Money registrar_credit_balance = std::string(locked_registrar_credit_result[0][1]);

      if(registrar_credit_balance < price && !enable_postpaid_operation)
      {
          //insufficient balance
          return false;
      }

      // save info about debt into credit
      Database::Result registrar_credit_transaction_result
          = conn.exec_params(
            "INSERT INTO registrar_credit_transaction "
                " (id, balance_change, registrar_credit_id) "
                " VALUES (DEFAULT, $1::numeric , $2::bigint) "
            " RETURNING id "
          , Database::query_param_list(Money("0") - price)(registrar_credit_id));

      if(registrar_credit_transaction_result.size() != 1)
      {
          throw std::runtime_error("charge_operation: registrar_credit_transaction failed");
      }

      unsigned long long registrar_credit_transaction_id = registrar_credit_transaction_result[0][0];

      // new record to invoice_operation
      //Database::Result invoice_operation_result =
      conn.exec_params(
          "INSERT INTO invoice_operation "
          " (id, object_id, registrar_id, operation_id, zone_id" //4
          " , crdate, quantity, date_from,  date_to "
          " , registrar_credit_transaction_id) "
          "  VALUES (DEFAULT, $1::bigint, $2::bigint, $3::bigint, $4::bigint "
          " , $5::timestamp, $6::integer, $7::date, $8::date "
          " , $9::bigint) "
          //" RETURNING id "
          , Database::query_param_list(object_id ? object_id : Database::QPNull)
          (registrar_id)(operation_id)(zone_id)
          (crdate)(quantity.get_string())(date_from)(date_to.is_special() ? Database::QPNull : Database::QueryParam(date_to))
          (registrar_credit_transaction_id)
          );

      return true;
  }

  /// charge registrar for requests over limit in request_fee_parameters
  // poll_msg_period_to specifies end of period for which it should be charged,
  //     it's the first day in month in case of charging for the previous month
  // there must be a poll message (type request fee) for the correct period
  // returns false in case insufficient balance (==applies only to prepaid operations) as in chargeDomain*()
  virtual bool chargeRequestFee(
          const Database::ID &registrar_id,
          date poll_msg_period_to)
  {
      TRACE("[CALL] LibFred::Invoicing::Manager::chargeRequestFee()");

      unsigned charge_zone_id;
      getRequestFeeParams(&charge_zone_id);

      Database::Connection conn = Database::Manager::acquire();
      DBSharedPtr ldb_dc_guard (new DB(conn));

      date poll_msg_period_from;

      if(poll_msg_period_to.day() == 1) {
          poll_msg_period_from = poll_msg_period_to - months(1);
      } else {
          poll_msg_period_from = date(poll_msg_period_to.year(), poll_msg_period_to.month(), 1);
      }

      boost::format msg("Charging registrar %1% for requests in period from %2% to %3%. ");
      msg % registrar_id % poll_msg_period_from % poll_msg_period_to;
      LOGGER.info(msg);

      // TODO handle NULL fields in this method
      LibFred::OperationContextCreator ctx;
      LibFred::Poll::RequestFeeInfoEvent rfi =
          LibFred::Poll::get_request_fee_info_message(ctx, registrar_id, ptime(poll_msg_period_to), "Europe/Prague");

      if(boost::date_time::c_local_adjustor<ptime>::utc_to_local (rfi.from)
              != ptime(poll_msg_period_from)) {
          throw std::runtime_error("Incorrect period_from in the poll message.");
      }

      // check if requests were already charged to this registrar and month combination

      Database::Result res = conn.exec_params(
        "SELECT io.id "
          "FROM invoice_operation io "
          "JOIN enum_operation eo ON eo.id=io.operation_id  "
        "WHERE eo.operation='GeneralEppOperation' "
          "AND registrar_id = $1::bigint "
          "AND date_trunc('month', date_from) <= date_trunc('month', $2::date) "
          "AND date_trunc('month', $2::date) <= date_trunc('month', date_to - interval '1 day') ",
            Database::query_param_list (registrar_id)
                                      (poll_msg_period_from)
                                      );
      if(res.size() > 0) {
          boost::format msg("Registrar %1% was already charged for requests in period overlapping %2% to %3%. ");
          msg % registrar_id % poll_msg_period_from % poll_msg_period_to;
          LOGGER.error(msg.str());
          // no charging - return true
          return true;
      }

      // was number of free requests exceeded
      if(rfi.used_count <= rfi.free_count) {
          boost::format msg("Registrar ID %1% has not exceeded request count limit set to %2%. ");
          msg % registrar_id % rfi.free_count;

          LOGGER.info(msg);
          return true;
      } else {
          unsigned long long paid_requests = rfi.used_count - rfi.free_count;
          boost::format msg(" Registrar ID %1% will be charged sum %2% for %3% requests over limit");
          msg % registrar_id % paid_requests % rfi.price;

          LOGGER.info(msg);

          return charge_operation_auto_price("GeneralEppOperation",//const std::string& operation
                charge_zone_id,//unsigned long long zone_id
                registrar_id, //unsigned long long registrar_id
                0, //unsigned long long object_id
                rfi.to - boost::posix_time::seconds(1), //boost::posix_time::ptime crdate
                boost::date_time::c_local_adjustor<ptime>::utc_to_local (rfi.from).date(), //boost::gregorian::date date_from
                boost::date_time::c_local_adjustor<ptime>::utc_to_local (rfi.to).date(), //boost::gregorian::date date_to
                Decimal(boost::lexical_cast<std::string>(paid_requests)) //unsigned long quantity - for renew in years
                );
      }
  }

  //count VAT from price with tax using coefficient - local CZ rules
  //returning vat rounded half-up to 2 decimal places
  Money count_dph(
          Money price, // CZK incl. vat
          Decimal vat_rate) // vat rate, for example 21 (%)
  {
      const Decimal vat_coeff = (Decimal("100") + vat_rate) / Decimal("100");
      Money vat = price - (price / vat_coeff);
      vat.round_half_up(2);

     LOGGER.debug(
         boost::format("count_dph price %1% vat rate %2% vat_coeff %3% vat %4%")
         % price % vat_rate % vat_coeff % vat);

     return vat;
  }

    /**
     *returns 0 in case of failure, id of invoice otherwise
     * type is int just because MakeNewInoviceAdvance returns it.
     */

// TODO what exceptions to throw and whether to log errors

unsigned long long  createDepositInvoice(boost::gregorian::date tax_date, unsigned long long zoneId
        , unsigned long long registrarId, Money price
        , boost::posix_time::ptime invoice_date //local timestamp
        , Money& out_credit
        )
{

    if (tax_date.is_special())
    {
        throw InvalidTaxDateFormat();
    }

    if (invoice_date.date() < tax_date ||
       (invoice_date.date() - tax_date > boost::gregorian::days(15)))
    {
        throw TaxDateTooOld();
    }

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
    Decimal vat_rate("0");
    // ratio for VAT calculation
    // price_without_vat = price_with_vat - (price_with_vat / vat_coeff)
    Decimal vat_coeff("0");

    Money vat_amount("0");
    Money total("0");

    if (pay_vat) {

        Database::Result vat_details = conn.exec_params(
                // clang-format off
                "SELECT vat "
                  "FROM price_vat "
                 "WHERE valid_to > ($1::TIMESTAMP AT TIME ZONE 'Europe/Prague' ) AT TIME ZONE 'UTC' "
                    "OR valid_to IS NULL "
                 "ORDER BY valid_to LIMIT 1",
                // clang-format on
                Database::query_param_list(tax_date));

        if(vat_details.size() > 1) {
            throw std::runtime_error("Multiple valid VAT values found.");
        } else if(vat_details.size() == 0) {
            throw std::runtime_error("No valid VAT value found.");
        }

        if(vat_details[0][0].isnull()) {
            vat_rate = Decimal("0");
        } else {
            vat_rate.set_string(vat_details[0][0]);
        }

        vat_amount = count_dph(price, vat_rate);
        total = price - vat_amount;
    } else {
        total = price;
    }
    out_credit = total;


    // get new invoice prefix and its type
    // TODO unclear thread safety in the old implementation
    Database::Result ip_res = conn.exec_params(
        "SELECT id, prefix  FROM invoice_prefix WHERE zone_id=$1::integer AND  typ=$2::integer AND year=$3::numeric FOR UPDATE",
        Database::query_param_list(zoneId)
                                (IT_DEPOSIT)
                                (tax_date.year())
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

    if(price.get_string().empty()) throw std::runtime_error("price empty");
    if(total.get_string().empty()) throw std::runtime_error("total empty");
    if(vat_amount.get_string().empty()) throw std::runtime_error("vat_amount empty");
    if(out_credit.get_string().empty()) throw std::runtime_error("credit empty");

    Database::Result curr_id = conn.exec_params(
            "INSERT INTO invoice (id, prefix, zone_id, invoice_prefix_id, registrar_id "
            ", crdate, taxDate, operations_price, vat, total, totalVAT, balance) VALUES "
            "(DEFAULT, $1::bigint, $2::bigint, $3::bigint, $4::bigint, "
            " ($5::timestamp AT TIME ZONE 'Europe/Prague' ) AT TIME ZONE 'UTC', "
            " $6::date, NULL, $7::numeric, "
            "$8::numeric(10,2), $9::numeric(10,2), $10::numeric(10,2)) " // total, totalVAT, balance
            " RETURNING id "
            , Database::query_param_list(inv_prefix)
                                (zoneId)
                                (inv_prefix_type)
                                (registrarId)
                                (invoice_date)
                                (tax_date)
                                (vat_rate)
                                (total.get_string())
                                (vat_amount.get_string())
                                (out_credit.get_string())
                                );

    if(curr_id.size() != 1 || curr_id[0][0].isnull()) {
        throw std::runtime_error("Couldn't create new invoice");
    }

    unsigned long long  invoiceId = curr_id[0][0];

    return invoiceId;
}

unsigned long long insert_account_invoice(
        unsigned long long registrar_id
        , unsigned long long zone_id
        , Money sum_op_price
        , boost::gregorian::date tax_date
        , boost::posix_time::ptime invoice_date //local timestamp, default = today
        )
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction tx(conn);

    if (invoice_date.date() < tax_date ||
       (invoice_date.date() - tax_date > boost::gregorian::days(15)))
    {
        throw std::runtime_error(
            "insert_account_invoice: invoice_date is more than"
            " 15 days later than tax_date");
    }

    Database::Result rvat_result = conn.exec_params(
            "SELECT vat FROM registrar WHERE id = $1::bigint"
            , Database::query_param_list(registrar_id));
    if(rvat_result.size() != 1 )
    {
        throw std::runtime_error(
                "insert_account_invoice: registrar vat not found");
    }
    bool rvat = rvat_result[0][0];

    Database::Result vat_details_result = conn.exec_params(
        "SELECT vat "
        " FROM price_vat "
        " WHERE valid_to > $1::date " // -- utc end of period date
        "   OR valid_to is NULL "
        " ORDER BY valid_to LIMIT 1 "
        , Database::query_param_list(tax_date));
    if (vat_details_result.size() != 1)
    {
        throw std::runtime_error(
                "insert_account_invoice: vat details not found");
    }

    Decimal vat_rate = std::string(vat_details_result[0][0]);

    Database::Result invoice_prefix_result = conn.exec_params(
        "SELECT id, prefix "
        " FROM invoice_prefix "
        " WHERE zone_id = $1::bigint AND typ = 1 " //typ = IT_ACCOUNT
        "   AND year = $2::numeric "
        " FOR UPDATE "
        , Database::query_param_list(zone_id)(tax_date.year()));

    if (invoice_prefix_result.size() != 1)
    {
        throw std::runtime_error(
                "insert_account_invoice: invoice_prefix not found");
    }

    unsigned long long invoice_prefix_id = invoice_prefix_result[0][0];
    unsigned long long prefix = invoice_prefix_result[0][1];

    conn.exec_params(
        "UPDATE invoice_prefix SET prefix = prefix + 1 WHERE id = $1::bigint"
        , Database::query_param_list(invoice_prefix_id));

    Database::Result invoice_result = conn.exec_params(
    "INSERT INTO invoice"
        " (id, prefix, zone_id, invoice_prefix_id, registrar_id "
        " , crdate, taxDate, operations_price, vat "
        " , total, totalVAT, balance) "
        " VALUES (DEFAULT, $1::bigint, $2::bigint, $3::bigint "
        " , $4::bigint, ($5::timestamp AT TIME ZONE 'Europe/Prague') AT TIME ZONE 'UTC' , $6::date, $7::numeric(10,2) "
        " , $8::numeric, 0, 0, 0) "
    " RETURNING id "
    , Database::query_param_list(prefix)(zone_id)(invoice_prefix_id)
    (registrar_id)(invoice_date)(tax_date)(sum_op_price.get_string())
    (rvat ? vat_rate.get_string() : Decimal("0").get_string())
    );

    if(invoice_result.size() != 1 )
    {
        throw std::runtime_error(
                "insert_account_invoice: insert invoice failed");
    }

    unsigned long long invoice_id = invoice_result[0][0];

    if(invoice_id == 0 )
    {
        throw std::runtime_error(
                "insert_account_invoice: insert invoice failed - invoice_id == 0");
    }

    tx.commit();
    return invoice_id;
}


unsigned long long create_account_invoice
    ( unsigned long long registrar_id
    , unsigned long long zone_id
    , boost::gregorian::date from_date //optional start of billing range
    , boost::gregorian::date to_date //end of billing range
    , boost::gregorian::date tax_date //default = to_date - may be different from to_date, affects selection of advance payments
    , boost::posix_time::ptime invoice_date //local timestamp, default = today - account invoice interval to date including to_date
    )
{
    Database::Connection conn = Database::Manager::acquire();

    if ((invoice_date.date() - to_date) > boost::gregorian::days(15) )
    {
        throw std::runtime_error(
            "create_account_invoice: invoice_date is more than"
            " 15 days later than to_date");
    }

    if(from_date.is_special())
    {


        boost::gregorian::date today
            = boost::posix_time::second_clock::local_time().date();
        if(to_date >= today)
        {
            throw std::runtime_error(
                        "create_account_invoice: todate >= today"
                        " - for custom account invoice range set fromdate");
        }

        //from_date = get_last_account_date(registrar, zone)
        Database::Result from_date_result = conn.exec_params(
            "SELECT date( todate + interval'1 day')  as fromdate "
            " FROM invoice_generation "
            " WHERE zone_id=$1::bigint "
            "  AND registrar_id =$2::bigint "
            " ORDER BY id DESC LIMIT 1 "
            , Database::query_param_list(zone_id)(registrar_id)
            );

        if (from_date_result.size() == 1)
        {
            from_date = from_date_result[0][0];
        }
        else
        {
            Database::Result from_date_registrar_result = conn.exec_params(
                "SELECT  fromdate  FROM registrarinvoice "// --for new registrar
                " WHERE zone=$1::bigint and registrarid=$2::bigint "
                , Database::query_param_list(zone_id)(registrar_id));
            if(from_date_registrar_result.size() != 1 )
            {
                throw std::runtime_error(
                        "create_account_invoice: from_date not found");
            }

            from_date = from_date_registrar_result[0][0];
        }
    }//if from_date not set

    if (from_date > to_date )
    {
        throw std::runtime_error(
            "create_account_invoice: from_date > to_date");
    }



    //ig = insert_invoice_generation(registar, zone, from_date, to_date)
    Database::Result invoice_generation_result = conn.exec_params(
        "INSERT INTO invoice_generation "
        " (id, fromdate, todate, registrar_id, zone_id, invoice_id) "
        " VALUES (DEFAULT, $1::date, $2::date, $3::bigint, $4::bigint, NULL ) "
        " RETURNING id "
    , Database::query_param_list(from_date)(to_date)(registrar_id)(zone_id));

    if(invoice_generation_result.size() != 1)
    {
        throw std::runtime_error(
                "create_account_invoice: invoice_generation failed");
    }

    unsigned long long invoice_generation_id = invoice_generation_result[0][0];

    // all operations without invoice_id set in invoice_operation with price from registrar_credit_transaction table
    //op_list = get_unaccounted_operations(registrar, zone, from_date, to_date)
    Database::Result unaccounted_operations_result = conn.exec_params(
        "SELECT io.id, io.operation_id, io.crdate, io.date_to, rct.balance_change * -1 as price "// --negative balance change is positive price
        " FROM invoice_operation io "
        " JOIN registrar_credit_transaction rct ON rct.id = io.registrar_credit_transaction_id "
        " WHERE io.ac_invoice_id IS NULL "
            " AND ((io.crdate AT TIME ZONE 'UTC') AT TIME ZONE 'Europe/Prague')::date >= $1::date "
            "  AND ((io.crdate AT TIME ZONE 'UTC') AT TIME ZONE 'Europe/Prague')::date <= $2::date "
            " AND io.registrar_id =  $3::bigint "
            " AND io.zone_id =  $4::bigint "
        " ORDER BY io.crdate, io.id"
    , Database::query_param_list(from_date)(to_date)(registrar_id)(zone_id));

    //if (sum_op_price = count_sum_price(op_list) == 0) return
    Money sum_op_price("0");
    for(unsigned long i = 0; i < unaccounted_operations_result.size(); ++i)
    {
        Money price = std::string(unaccounted_operations_result[i][4]);//price
        sum_op_price += price;
    }

    if(sum_op_price == Money("0")) return 0;

    //aci = insert_account_invoice(registrar, zone, sum_op_price, tax_date, invoice_date)
    unsigned long long aci = insert_account_invoice(
             registrar_id
            , zone_id
            ,  sum_op_price
            , tax_date
            , invoice_date //default = today
            );

    if(aci == 0 )
    {
        throw std::runtime_error(
                "create_account_invoice: insert_account_invoice failed - invoice_id == 0");
    }


    //update invoice_generation
    conn.exec_params(
        "UPDATE invoice_generation SET invoice_id = $1::bigint "
        " WHERE id = $2::bigint"
        , Database::query_param_list(aci)(invoice_generation_id));

    //adi_list = get_advance_invoices(registrar, zone, tax_date) , 0 as credit_change
    Database::Result advance_invoices_result = conn.exec_params(
    "SELECT i.id, i.balance FROM invoice i JOIN invoice_prefix ip ON i.invoice_prefix_id = ip.id "
        " WHERE ip.typ = 0 " //--IT_ADVANCE
        " AND i.zone_id = $1::bigint AND i.registrar_id = $2::bigint AND i.balance > 0 AND i.taxdate <= $3::date "
        " ORDER BY i.crdate, i.id "
    , Database::query_param_list(zone_id)(registrar_id)(tax_date));
    std::vector<AdvanceInvoice> adi_list;
    //init adi_list
    for (std::size_t i = 0; i < advance_invoices_result.size(); ++i)
    {
        AdvanceInvoice adi;
        adi.invoice_id = advance_invoices_result[i][0];//invoice id
        adi.balance = std::string(advance_invoices_result[i][1]);//balance
        adi.credit_change = Money("0");
        adi_list.push_back(adi);
    }

    Money price_left ("0"); //price of operations not paid ahead

    //for op in op_list
    for(unsigned long i = 0; i < unaccounted_operations_result.size(); ++i)
    {
        unsigned long long invoice_operation_id = unaccounted_operations_result[i][0];//invoice_operation.id
        //update_operation_set_account_invoice - add operation to new account invoice
        conn.exec_params(
            "UPDATE invoice_operation SET ac_invoice_id=$1::bigint WHERE id = $2::bigint"
            , Database::query_param_list(aci)//created account invoice id
            (invoice_operation_id));
        Money price_to_pull_off = std::string(unaccounted_operations_result[i][4]);//price

        //compute from which advance invoice was paid
        for (std::size_t i = 0; i < adi_list.size(); ++i)
        {
            if(adi_list[i].balance == Money("0")) continue;

            Money partial_charge = (price_to_pull_off <= adi_list[i].balance)
                    ? price_to_pull_off : adi_list[i].balance;

            adi_list[i].balance -= partial_charge;
            adi_list[i].credit_change += partial_charge;
            price_to_pull_off -= partial_charge;

            //insert_invoice_operation_charge_map
            conn.exec_params("INSERT INTO invoice_operation_charge_map "
                 " (invoice_operation_id, invoice_id, price) "
                 " VALUES ($1::bigint, $2::bigint, $3::numeric(10,2)) "
                , Database::query_param_list(invoice_operation_id)
                (adi_list[i].invoice_id)(partial_charge.get_string()));

            if (price_to_pull_off == Money("0")) break;
        }//for adi_list

        if(price_to_pull_off > Money("0"))
        {
            //insert_invoice_operation_charge_map
            conn.exec_params("INSERT INTO invoice_operation_charge_map "
                 " (invoice_operation_id, invoice_id, price) "
                 " VALUES ($1::bigint, $2::bigint, $3::numeric(10,2)) "
                , Database::query_param_list(invoice_operation_id)
                (aci)(price_to_pull_off.get_string()));

        }

        price_left += price_to_pull_off;
    }//for operations

    for (std::size_t i = 0; i < adi_list.size(); ++i)
    {
        if(adi_list[i].credit_change > Money("0"))
        {
            //update_invoice_balance(adi)
            conn.exec_params("UPDATE invoice SET balance = $1::numeric(10,2) WHERE id = $2::bigint"
                , Database::query_param_list(adi_list[i].balance.get_string())(adi_list[i].invoice_id));

            //insert_invoice_credit_payment_map(aci, adi)
            conn.exec_params("INSERT INTO invoice_credit_payment_map "
                " (ac_invoice_id, ad_invoice_id, credit, balance) "
                " VALUES($1::bigint, $2::bigint, $3::numeric(10,2), $4::numeric(10,2)) "
                , Database::query_param_list(aci)(adi_list[i].invoice_id)
                (adi_list[i].credit_change.get_string())
                (adi_list[i].balance.get_string()));
        }
    }

    //update_account_invoice(aci,price_left)
    conn.exec_params("UPDATE invoice "
      " SET balance = $1::numeric(10,2), total = $1::numeric(10,2), totalvat = "
      " (SELECT ($1::numeric * vat / 100::numeric)::numeric(10,2) FROM invoice WHERE id = $2::bigint) "
      " WHERE id = $2::bigint "
    , Database::query_param_list(price_left.get_string()) (aci));

    return aci;
}



void createAccountInvoices(
        const std::string& zone_fqdn
        , boost::gregorian::date taxdate
        , boost::gregorian::date fromdate
        , boost::gregorian::date todate //including end of range
        , boost::posix_time::ptime invoicedate //timestamp in local time
        )
{
    Logging::Context ctx("createAccountInvoices");
    try
    {
        if(zone_fqdn.empty()) throw std::runtime_error("createAccountInvoices: zone_fqdn is empty");
        if(todate.is_special()) throw std::runtime_error("createAccountInvoices: todate not set");

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        std::string taxdateStr(boost::gregorian::to_iso_extended_string(taxdate));
        std::string fromdateStr (boost::gregorian::to_iso_extended_string(fromdate));
        std::string todateStr(boost::gregorian::to_iso_extended_string(todate));

        Database::Result res = conn.exec_params(
           "SELECT DISTINCT oq.registrar_id, oq.zone_id "
           " , oq.fromdate_ig, oq.i_fromdate, oq.i_todate "
           " FROM (SELECT r.id as registrar_id, z.id as zone_id "
           " , i.fromdate as i_fromdate, i.todate as i_todate "
           " , (SELECT $2::date ) as p_fromdate "
           " , (SELECT $3::date ) as p_todate "
           " , (SELECT date( todate + interval'1 day')  as fromdate "
           " FROM invoice_generation "
           " WHERE zone_id=z.id "
           " AND registrar_id =r.id "
           " ORDER BY id DESC LIMIT 1 ) as fromdate_ig "
           " FROM registrar r, registrarinvoice i, zone z "
           " WHERE r.id=i.registrarid AND r.system=false "
           " AND i.zone=z.id AND z.fqdn=$1::text) as oq "
           " WHERE (oq.i_fromdate <= coalesce(oq.p_fromdate,oq.fromdate_ig, oq.i_fromdate ) "
           "   AND (oq.i_todate >= coalesce(oq.p_fromdate,oq.fromdate_ig, i_fromdate) "
           "   OR oq.i_todate is null)) "
           " OR (oq.i_fromdate <= oq.p_todate "
           " AND (oq.i_todate >= oq.p_todate OR oq.i_todate is null)) "
           " OR (oq.i_fromdate > coalesce(oq.p_fromdate,oq.fromdate_ig, oq.i_fromdate ) "
           " AND (oq.i_todate < oq.p_todate)) "
          , Database::query_param_list (zone_fqdn)(fromdate.is_special() ? Database::QPNull : fromdate )(todate)
          );

        LOGGER.debug ( boost::format("ManagerImpl::createAccountInvoices"
                " zone_fqdn %1%  taxdateStr %2% fromdateStr %3% todateStr %4%")
        % zone_fqdn % taxdateStr % fromdateStr % todateStr);


        for(std::size_t i = 0; i < res.size(); ++i)
        {
            unsigned long long regID = res[i][0];//regID
            unsigned long long zoneID =res[i][1];//zoneID

            try
            {
            unsigned long long invoiceID =
                    create_account_invoice
                    ( regID, zoneID
                    , fromdate, todate
                    , taxdate //default = to_date - may be different from to_date, affects selection of advance payments
                    , invoicedate//invoice_date //default = today - account invoice interval to date including to_date
                    );
                    //MakeFactoring(regID, zoneID, timestampStr,taxdateStr);
            LOGGER.notice(boost::format(
             "Vygenerovana fa invoiceID %1% pro regID %2% zoneID %3% taxdateStr %4%")
             % invoiceID % regID % zoneID % taxdateStr);
            }
            catch (const std::exception& ex)
            {
                std::string err_msg = str(boost::format(
                        "createAccountInvoices regID %1% zoneID %2% taxdateStr %3% : %4%")
                         % regID % zoneID % taxdateStr % ex.what());
                LOGGER.error(err_msg);
                std::cerr << "Error: " << err_msg << std::endl;
            }
        }//for i

        tx.commit();
    }//try
    catch( std::exception &ex)
    {
        LOGGER.error ( boost::format("createAccountInvoices failed: %1% ") % ex.what());
        throw std::runtime_error(std::string("createAccountInvoices failed: ") + ex.what());
    }
    catch(...)
    {
        LOGGER.error("createAccountInvoices failed.");
        throw std::runtime_error("createAccountInvoices failed");
    }
}//createAccountInvoices

// close invoice to registar handle for zone make taxDate to the todateStr
void createAccountInvoice(
        const std::string& registrarHandle
        , const std::string& zone_fqdn
        , boost::gregorian::date taxdate
        , boost::gregorian::date fromdate //optional start of range
        , boost::gregorian::date todate //including end of range
        , boost::posix_time::ptime invoicedate //timestamp in local time
        )
{
    Logging::Context ctx("createAccountInvoice");
    try
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        std::string taxdateStr(boost::gregorian::to_iso_extended_string(taxdate));
        std::string fromdateStr (boost::gregorian::to_iso_extended_string(fromdate));
        std::string todateStr(boost::gregorian::to_iso_extended_string(todate));

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

                LOGGER.debug ( boost::format("ManagerImpl::createAccountInvoice"
                        " regID %1%  zone %2% taxdateStr %3%  timestampStr %4%")
                % regID % zone % taxdateStr % timestampStr );

                // make invoice
                create_account_invoice
                    ( regID, zone
                    , fromdate, todate
                    , taxdate //default = to_date - may be different from to_date, affects selection of advance payments
                    , invoicedate //invoice_date //default = today - account invoice interval to date including to_date
                    );
                //MakeFactoring(regID, zone, timestampStr, taxdateStr);

            }
            else
            {
                std::string msg = str( boost::format("unknown zone %1% \n") % zone_fqdn);
                LOGGER.error(msg  );
                throw std::runtime_error(msg);
            }

        }
        else
        {
            std::string msg = str( boost::format("unknown registrarHandle %1% ") % registrarHandle );
            LOGGER.error( msg );
            throw std::runtime_error(msg);
        }

        tx.commit();
    }//try
    catch( std::exception &ex)
    {
      LOGGER.error ( boost::format("createAccountInvoice failed: %1% ") % ex.what());
      throw std::runtime_error(std::string("createAccountInvoice failed: ") + ex.what());
    }
    catch(...)
    {
      LOGGER.error("createAccountInvoice failed.");
      throw std::runtime_error("createAccountInvoice failed");
    }
}//createAccountInvoice

std::vector<unpaid_account_invoice> find_unpaid_account_invoices(
        unsigned long long registrar_id
        , unsigned long long zone_id)
{
    std::vector<unpaid_account_invoice> ret;

    Database::Connection conn = Database::Manager::acquire();

    Database::Result unpaid_account_invoices_result = conn.exec_params
        ("SELECT i.id, i.balance, i.vat "
        " FROM invoice i "
           " JOIN invoice_prefix ip ON i.invoice_prefix_id = ip.id AND ip.typ = 1 "   // account invoice prefix typ = 1
        " WHERE i.balance > 0 " // unpaid account balance is positive number
           " AND i.registrar_id = $1::bigint"
           " AND i.zone_id = $2::bigint "
        " ORDER BY i.id "
        , Database::query_param_list(registrar_id)(zone_id));

    for(unsigned i = 0 ; i < unpaid_account_invoices_result.size(); ++i)
    {
        unpaid_account_invoice uai;
        uai.id = unpaid_account_invoices_result[i][0];
        uai.balance = std::string(unpaid_account_invoices_result[i][1]);
        uai.vat = std::string(unpaid_account_invoices_result[i][2]);
        ret.push_back(uai);
    }

    return ret;
}//find_unpaid_account_invoices


    Money lower_account_invoice_balance_by_paid_amount( //returning paid amount without vat for credit transaction
        Money paid  //paid amount with vat
        , Decimal invoice_vat_rate //vat rate of account invoice
        , unsigned long long invoice_id) //account invoice id
    {
        Logging::Context ctx("lower_account_invoice_balance_by_paid_amount");
        Database::Connection conn = Database::Manager::acquire();

        //check if account invoice
        Database::Result check_inv_type =
        conn.exec_params("select ip.typ from invoice i "
            " join invoice_prefix ip on i.invoice_prefix_id = ip.id "
            " where i.id = $1::bigint"
            , Database::query_param_list(invoice_id));

        const Money vat = count_dph(paid, invoice_vat_rate);
        const Money balance_change = Money(paid - vat);

        //if account invoice before invoice_type table
        if (check_inv_type.size() > 0 && std::string(check_inv_type[0][0]).compare("1") == 0)
        {
            //update invoice set balance = balance - price
            conn.exec_params(
            "UPDATE invoice SET balance = balance - $1::numeric(10,2) WHERE id = $2::bigint"
            , Database::query_param_list(balance_change.get_string())(invoice_id));
        }
        else
        {
            throw std::runtime_error(
                    "lower_account_invoice_balance_by_paid_amount: not an account invoice");
        }
        return balance_change;
    }//lower_account_invoice_balance_by_paid_amount

    Money zero_account_invoice_balance(unsigned long long invoice_id) //account invoice id
    {
        Logging::Context ctx("zero_account_invoice_balance");
        Database::Connection conn = Database::Manager::acquire();

        //check if account invoice and get rest of the balance
        Database::Result check_inv_type =
            conn.exec_params("SELECT ip.typ, i.balance FROM invoice i "
            " JOIN invoice_prefix ip ON i.invoice_prefix_id = ip.id "
            " WHERE i.id = $1::bigint "
            " FOR UPDATE OF i"
            , Database::query_param_list(invoice_id));

        //if account invoice before invoice_type table
        if (check_inv_type.size() > 0 && std::string(check_inv_type[0][0]).compare("1") == 0)
        {
            //update invoice set balance = 0
            conn.exec_params(
            "UPDATE invoice SET balance = 0 WHERE id = $1::bigint"
            , Database::query_param_list(invoice_id));
        }
        else
        {
            throw std::runtime_error(
                    "zero_account_invoice_balance: not an account invoice");
        }
        return Money(std::string(check_inv_type[0][1]));
    }//zero_account_invoice_balance


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
  Decimal vatRate; ///< vatRate of this advance invoice
  Money vat; ///< total vat - approx. (price * vatRate/100)
public:
  PaymentImpl(const PaymentImpl* p) :
    price(p->price), vatRate(p->vatRate), vat(p->vat) {
  }
  PaymentImpl(Money _price, Decimal _vatRate, Money _vat) :
    price(_price), vatRate(_vatRate), vat(_vat) {
  }
  virtual Money getPrice() const
  {
      Logging::Context ctx("PaymentImpl::getPrice");
      try
      {
          if(price.is_special())
              throw std::runtime_error("price is special");
          return price;
      }//try
      catch(const std::exception& ex)
      {
          LOGGER.debug(ex.what());
          throw;
      }

  }
  virtual Decimal getVatRate() const
  {
    Logging::Context ctx("PaymentImpl::getVatRate");
    try
    {
        if(vatRate.is_special())
            throw std::runtime_error("vatRate is special");
        return vatRate;
    }//try
    catch(const std::exception& ex)
    {
        LOGGER.debug(ex.what());
        throw;
    }
  }
  virtual Money getVat() const
  {
      Logging::Context ctx("PaymentImpl::getVat");
      try
      {
          if(vat.is_special())
              throw std::runtime_error("vat is special");
          return vat;
      }//try
      catch(const std::exception& ex)
      {
          LOGGER.debug(ex.what());
          throw;
      }
  }
  virtual Money getPriceWithVat() const
  {
      Logging::Context ctx("PaymentImpl::getPriceWithVat");
      Money ret;
      try
      {
          ret = price + vat;
      }//try
      catch(const std::exception& ex)
      {
          LOGGER.debug(ex.what());
          throw;
      }
      return ret;
  }
  bool operator==(Decimal _vatRate) const
  {
      Logging::Context ctx("PaymentImpl::operator==");
      try
      {
          return vatRate == _vatRate;
      }//try
      catch(const std::exception& ex)
      {
          LOGGER.debug(ex.what());
          throw;
      }
  }
  void add(const PaymentImpl *p)
  {
    Logging::Context ctx("PaymentImpl::add");
    try
    {
        price += p->price;
        vat += p->vat;
    }//try
    catch(const std::exception& ex)
    {
        LOGGER.debug(ex.what());
        throw;
    }
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
  PaymentSourceImpl(Money _price, Decimal _vat_rate, Money _vat,
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
  virtual Money getCredit() const
  {
      Logging::Context ctx("PaymentSourceImpl::getCredit");
      try
      {
          if(credit.is_special())
              throw std::runtime_error("credit is special");
          return credit;
      }//try
      catch(const std::exception& ex)
      {
          LOGGER.debug(ex.what());
          throw;
      }
  }
  virtual TID getId() const
  {
    return id;
  }
  virtual Money getTotalPrice() const
  {
      Logging::Context ctx("PaymentSourceImpl::getTotalPrice");
      try
      {
          if(totalPrice.is_special())
              throw std::runtime_error("totalPrice is special");
          return totalPrice;
      }//try
      catch(const std::exception& ex)
      {
          LOGGER.debug(ex.what());
          throw;
      }
  }
  virtual Money getTotalVat() const
  {
      Logging::Context ctx("PaymentSourceImpl::getTotalVat");
      try
      {
          if(totalVat.is_special())
              throw std::runtime_error("totalVat is special");
          return totalVat;
      }//try
      catch(const std::exception& ex)
      {
          LOGGER.debug(ex.what());
          throw;
      }
  }
  virtual Money getTotalPriceWithVat() const
  {
      Logging::Context ctx("PaymentSourceImpl::getTotalPriceWithVat");
      try
      {
          if(totalPrice.is_special())
              throw std::runtime_error("totalPrice is special");
          if(totalVat.is_special())
              throw std::runtime_error("totalVat is special");
          return totalPrice + totalVat;
      }//try
      catch(const std::exception& ex)
      {
          LOGGER.debug(ex.what());
          throw;
      }
  }
  virtual ptime getCrTime() const
  {
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
  date fromDate; ///< fromdate of domain
  date exDate; ///< exdate of domain 
  PaymentActionType action; ///< type of action that is subject of payment
  unsigned unitsCount; ///< number of months to expiration of domain
  Money pricePerUnit; ///< copy of price from price list
  TID objectId; ///< id of object affected by payment action
public:
  /// init content from sql result (ignore first column)
  PaymentActionImpl(Money _price, Decimal _vat_rate, Money _vat,
                    std::string& _object_name, ptime _action_time, date _fromdate, date _exdate,
                    PaymentActionType _type, unsigned _units, Money _price_per_unit, TID _id) :
                      PaymentImpl(_price, _vat_rate, _vat),
                      objectName(_object_name),
                      actionTime(_action_time),
                      fromDate(_fromdate),
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
  virtual date getFromDate() const {
    return fromDate;
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
            case PAT_REQUESTS_OVER_LIMIT:
                return "REQ";
                break;
            default:
                return "UNKNOWN";
                break;
        }

        return "UNKNOWN";
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

namespace {

bool do_use_coef(boost::posix_time::ptime crtime, Decimal vat_rate, Money total_without_vat, Money total_vat)
{
  static const auto use_coef_before =
          boost::posix_time::ptime(
                  boost::gregorian::date(2019, boost::gregorian::Oct, 1),
                  boost::posix_time::hours(-2)); // CEST

  if (crtime < use_coef_before)
  {
      return true;
  }

  const auto total_possibly_using_coef = total_without_vat + total_vat;
  const auto total_using_math = total_without_vat + total_without_vat * (vat_rate / Decimal("100"));
  static const auto round_err_max = Money("0.01");
  const auto total_definitely_used_coef = (total_possibly_using_coef - total_using_math).abs() > round_err_max;

  return total_definitely_used_coef;
}

} // namespace LibFred::Invoicing::{anonymous}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//   InvoiceImpl
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
/// implementation of interface Invoice


class InvoiceImpl : public LibFred::CommonObjectImpl,
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
  Decimal vatRate;
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
              TID _registrar, Money _credit, Money _price, Decimal _vatRate, Money _total,
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
                                       man(_manager),
                                       id(_id) {
      LOGGER.debug ( boost::format(
              "InvoiceImpl _id: %1% _zone: %2% _zoneName: %3% _crTime: %4% _taxDate: %5%"
              " _accountPeriod: %6% _type: %7% _number: %8%"
              " _registrar: %9% _credit: %10% _price: %11% _vatRate: %12% _total: %13%"
              " _totalVAT: %14% _filePDF: %15% _fileXML: %16% _varSymbol: %17%"
              //" _client: %18% _filepdf_name: %19%  _filexml_name: %20%"
              //" _manager: %21%"
              )
      % _id % _zone % _zoneName % _crTime % _taxDate
      % _accountPeriod % _type %  _number
      % _registrar % (_credit.is_special() ? std::string(" null ") : _credit.get_string()) //credit is null for account invoices
      % (_price.is_special() ? std::string(" null ") : _price.get_string())
      % _vatRate % _total
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
  Decimal getVatRate() const {
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
          LOGGER.error ( boost::format("storeFile failed: %1% ") % ex.what());
          throw std::runtime_error(std::string("storeFile failed: ") + ex.what());
      }
      catch(...)
      {
          LOGGER.error("storeFile failed.");
          throw std::runtime_error("storeFile failed");
      }
  }//storeFile

  /// export invoice using given exporter
  void doExport(Exporter *exp) {
    exp->doExport(this);
  }
  /// initialize list of actions from sql result

  void addAction(Database::Row::Iterator& _col) {
    std::string _price = std::string(*_col);
    std::string _vat_rate = std::string( *(++_col));
    std::string        object_name = *(++_col);
    Database::DateTime action_time = *(++_col);
    Database::Date     fromdate      = *(++_col);
    Database::Date     exdate      = *(++_col);
    int operation_id =  *(++_col);
    PaymentActionType  type        = PaymentActionType (operation_id - 1); //PaymentActionType = id -1

    unsigned           units          = *(++_col); 
    std::string _price_per_unit = std::string(*(++_col));
    Database::ID       id             = *(++_col);

    Money    price(_price);
    Decimal    vat_rate(_vat_rate);
    Money price_per_unit(_price_per_unit);

    LOGGER.debug(
        boost::format(
        "addAction _price %1% _vat_rate %2% object_name %3% PaymentActionType %4%"
        " units %5% _price_per_unit %6% id %7%")
        % _price
        % _vat_rate
        % object_name
        % type
        % units
        % _price_per_unit
        % id
    );

    const constexpr bool use_coef = false;
    PaymentActionImpl *new_action = new PaymentActionImpl(price,
                                                          vat_rate,
                                                          man->countVAT(price, vat_rate, use_coef),
                                                          object_name,
                                                          action_time,
                                                          fromdate,
                                                          exdate,
                                                          type,
                                                          units,
                                                          price_per_unit,
                                                          id);
    actions.push_back(new_action);
  }
  /// initialize list of sources from sql result

  void addSource(Database::Row::Iterator& _col) {
    std::string _price       = std::string(*_col);
    std::string _vat_rate = std::string( *(++_col));
    unsigned long long number   = *(++_col);  
    std::string _credit = std::string(*(++_col));
    Database::ID id             = *(++_col);
    std::string _total_price = std::string(*(++_col));
    std::string _total_vat   = std::string(*(++_col));
    Database::DateTime crtime   = *(++_col);

    Money price(_price);
    Decimal vat_rate(_vat_rate);
    Money credit(_credit);
    Money total_price(_total_price);
    Money total_vat(_total_vat);

    const bool use_coef = do_use_coef(crtime.get(), vat_rate, total_price, total_vat);

    LOGGER.debug(
        boost::format(
        "addSource _price %1% _vat_rate %2% number %3% _credit %4%"
        " _total_price %5% _total_vat %6% id %7% use_coef %8%")
        % _price
        % _vat_rate
        % number
        % _credit
        % _total_price
        % _total_vat
        % id
        % use_coef
    );

    PaymentSourceImpl *new_source = new PaymentSourceImpl(price,
                                                          vat_rate,
                                                          man->countVAT(price, vat_rate, use_coef),
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
    virtual Decimal getVat() const {
        return getVatRate();
    };
    virtual Money getTotalVat() const {
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
                          "Milešovská 1136/5",
                          "Praha 3",
                          "130 00",
                          "CZ",
                          "67985726",
                          "CZ67985726",
                          "SpZ: odb. občanskopr. agend Magist. hl. m. Prahy, č. ZS/30/3/98",
                          "CZ.NIC, z.s.p.o., Milešovská 1136/5, 130 00 Praha 3",
                          "www.nic.cz",
                          "podpora@nic.cz",
                          "+420 222 745 111",
                          "",
                           1);
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//   ExporterXML
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#define TAGSTART(tag) "<"#tag">"
#define TAGEND(tag) "</"#tag">"
#define TAG(tag,f) TAGSTART(tag) \
                       << "<![CDATA[" << f << "]]>" << TAGEND(tag)
#define OUTMONEY(f) f.get_string(".2f")
// builder that export xml of invoice into given stream
class ExporterXML : public Exporter {
  std::ostream& out;
  bool xmlDec; ///< whether to include xml declaration 
  static const constexpr char* annual_partitioning_query = \
 "SELECT moo.year, "
       "moo.adi_vat_rate, "
       "SUM(moo.price_novat)::NUMERIC(10,2) AS price_novat, "
       "SUM(moo.price_vat)::NUMERIC(10,2) AS price_vat, "
       "SUM(moo.price_vat)::NUMERIC(10,2) - SUM(moo.price_novat)::NUMERIC(10,2) AS vat "
  "FROM ( "
        "SELECT baz.year, "
               "baz.adi_vat_rate, "
               "baz.adi_vat_alg, "
               "baz.price_novat, "
               "CASE WHEN baz.adi_vat_alg = 'coef' "
                    "THEN baz.price_novat * (1 / (1 - (SELECT koef FROM price_vat WHERE vat = baz.adi_vat_rate))) "
                    "WHEN baz.adi_vat_alg = 'math' "
                    "THEN baz.price_novat * (1 + (baz.adi_vat_rate / 100)) "
                    "WHEN baz.adi_vat_alg = 'novat' "
                    "THEN 0 "
               "end AS price_vat "
          "FROM ( "
                "SELECT bar.year, bar.adi_vat_rate, bar.adi_vat_alg, SUM(bar.price_novat) AS price_novat "
                  "FROM ( "
                        "SELECT foo.invoice_operation_id, "
                               "foo.ad_invoice_id, "
                               "foo.adi_vat_rate, "
                               "foo.adi_vat_alg, "
                               "foo.year, "
                               "foo.price * (foo.days_per_year / (SUM(foo.days_per_year) OVER (partition by foo.invoice_operation_id, foo.ad_invoice_id))) AS price_novat "
                          "FROM ( "
                                "SELECT iocm.invoice_operation_id, "
                                       "iocm.invoice_id AS ad_invoice_id, "
                                       "iocm.price, "
                                       "adi.vat AS adi_vat_rate, "
                                       "CASE WHEN adi.vat > 0 "
                                            "THEN CASE WHEN adi.crdate < (('2019-10-01 00:00:00' AT TIME ZONE 'UTC') AT TIME ZONE 'Europe/Prague') "
                                                           "or abs(((adi.total + adi.totalvat) * (1 - 1/(1 + adi.vat/100)))::NUMERIC(10,2) - adi.totalvat) > 0.01 "
                                                      "THEN 'coef' "
                                                      "ELSE 'math' "
                                                 "end "
                                            "ELSE 'novat' "
                                       "end AS adi_vat_alg, "
                                       "EXTRACT(YEAR FROM generate_series(io.date_from + '1 day'::INTERVAL, io.date_to, '1 day'::INTERVAL)) AS year, "
                                       "COUNT(*) AS days_per_year "
                                  "FROM invoice_operation io "
                                  "JOIN invoice_operation_charge_map iocm ON iocm.invoice_operation_id = io.id "
                                  "JOIN invoice adi ON adi.id = iocm.invoice_id "
                                 "WHERE io.ac_invoice_id = $1::BIGINT "
                                 "GROUP BY 1,2,3,4,5,6 "
                                ") AS foo "
                       ") AS bar "
                   "GROUP BY 1,2,3 "
                   "ORDER BY 1,2,3 "
               ") AS baz "
       ") AS moo "
 "group by 1,2;";

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

      //date check
      if(i->getCrTime().date().is_special())
          throw std::runtime_error("doExport invoice crtime date is special");
      if(i->getTaxDate().is_special())
          throw std::runtime_error("doExport invoice taxdate is special");
      if (i->getType() == IT_ACCOUNT)
      {
          if(i->getAccountPeriod().begin().is_special())
              throw std::runtime_error("doExport invoice period_from is special");
          if(i->getAccountPeriod().end().is_special())
              throw std::runtime_error("doExport invoice period_to is special");
      }

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


      Database::Connection conn = Database::Manager::acquire();

      Database::Result result =
              conn.exec_params(
                      annual_partitioning_query,
                      Database::query_param_list(i->getId()));

      using MoneyRecord = std::array<Money, 3>;
      using YearRecord = std::map<unsigned, MoneyRecord>;
      using VatrateRecord = std::map<Decimal, YearRecord>;
      VatrateRecord records;

      for (std::size_t i = 0; i < result.size(); ++i) {
          const auto year = static_cast<unsigned>(result[i][0]);
          const auto vat_rate = Decimal(static_cast<std::string>(result[i][1]));
          const auto price_without_vat = Money(static_cast<std::string>(result[i][2]));
          const auto price_with_vat = Money(static_cast<std::string>(result[i][3]));
          const auto vat = Money(static_cast<std::string>(result[i][4]));
          if (records[vat_rate][year][0].is_special())
          {
              records[vat_rate][year][0] = Money("0");
              records[vat_rate][year][1] = Money("0");
              records[vat_rate][year][2] = Money("0");
          }
          records[vat_rate][year][0] += price_without_vat;
          records[vat_rate][year][1] += price_with_vat;
          records[vat_rate][year][2] += vat;
      }

      bool added_price = false;
      for (const auto& record : records) {
        const auto vat_rate = record.first;
        Money price_without_vat = Money("0");
        Money price_with_vat = Money("0");
        Money vat = Money("0");
        for (const auto& year_record : record.second) {
            price_without_vat += year_record.second[0];
            price_with_vat += year_record.second[1];
            vat += year_record.second[2];
        }

        if(added_price == false && (i->getVatRate() == vat_rate) && (i->getType() == IT_ACCOUNT))
        {//add ac invoice to vat details

            out << TAGSTART(entry)
            << TAG(vatperc, vat_rate)
            << TAG(basetax,OUTMONEY(i->getTotal()))
            << TAG(vat,OUTMONEY(i->getTotalVAT()))
            << TAG(total,OUTMONEY( (price_with_vat + i->getTotal() + i->getTotalVAT()) ))
            << TAG(totalvat,OUTMONEY( (vat + i->getTotalVAT()) ))
            << TAG(paid,OUTMONEY(price_with_vat))
            << TAG(paidvat,OUTMONEY(vat))
            << TAGSTART(years);

            added_price = true;
        }
        else
        {
            out << TAGSTART(entry)
            << TAG(vatperc, vat_rate)
            << TAG(basetax,OUTMONEY(price_without_vat))
            << TAG(vat,OUTMONEY(vat))
            << TAG(total,OUTMONEY(price_with_vat))
            << TAG(totalvat,OUTMONEY(vat))
            << TAG(paid,OUTMONEY(price_with_vat))
            << TAG(paidvat,OUTMONEY(vat))
            << TAGSTART(years);
        }

        for (const auto& year_record : records[vat_rate])
        {
            const auto year = year_record.first;
            const auto price_without_vat = year_record.second[0];
            const auto price_with_vat = year_record.second[1];
            const auto vat = year_record.second[2];

            out << TAGSTART(entry)
            << TAG(year, year)
            << TAG(price,OUTMONEY(price_without_vat))
            << TAG(vat,OUTMONEY(vat))
            << TAG(total,OUTMONEY(price_with_vat))
            << TAGEND(entry);
        }

        out << TAGEND(years)
        << TAGEND(entry);
      }//for payment count

      if((added_price == false) && (i->getTotal() != Money("0")) && (i->getType() == IT_ACCOUNT))
      {//add ac invoice to vat details
        out << TAGSTART(entry)
        << TAG(vatperc,i->getVatRate())
        << TAG(basetax,OUTMONEY(i->getTotal()))
        << TAG(vat,OUTMONEY(i->getTotalVAT()))
        << TAG(total,OUTMONEY( (i->getTotal() + i->getTotalVAT()) ))
        << TAG(totalvat,OUTMONEY(i->getTotalVAT()))
        << TAG(paid,OUTMONEY(Money("0")))
        << TAG(paidvat,OUTMONEY(Money("0")))
        << TAGEND(entry);
        added_price = true;
       }

      out << TAGEND(vat_rates)
      << TAGSTART(sumarize)
      << TAG(total,OUTMONEY(i->getPrice()))
      << TAG(paid,
          OUTMONEY((i->getType() == IT_DEPOSIT ? Money("0") : ((i->getPrice() - i->getTotal() )*Money("-1")) )))
      << TAG(to_be_paid,OUTMONEY( (i->getType() == IT_DEPOSIT ? Money("0") : i->getTotal() + i->getTotalVAT()) ))
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
              (pa->getAction() == PAT_CREATE_DOMAIN ? "RREG"
              : (pa->getAction() == PAT_RENEW_DOMAIN ? "RUDR"
                : (pa->getAction() == PAT_REQUESTS_OVER_LIMIT ? "REPP"
                  : (pa->getAction() == PAT_FINE ? "RPOK"
                    : (pa->getAction() == PAT_FEE ? "RPOP"
                      : "RUNK")
                    )
                  )
                )
              )
             )//TAG code
          << TAG(timestamp,pa->getActionTime());
          if (!pa->getExDate().is_special())
          out << TAG(expiration,pa->getExDate());
          out << TAG(count,pa->getUnitsCount())
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
        std::unique_ptr<Document::Generator> gPDF(
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

        LOGGER.debug ( boost::format("ExporterArchiver::doExport pdf file id: %1% ") % filePDF);

        // create generator for XML
        std::unique_ptr<Document::Generator> gXML(
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
        LOGGER.error("Exception in ExporterArchiver::doExport.");
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

  class ListImpl : public LibFred::CommonListImpl,
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
      std::unique_ptr<Database::Filters::Iterator> fit(_uf.createIterator());
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
        LOGGER.error("wrong filter passed for reload!");
        return;
      }

      id_query.order_by() << "id DESC";
      id_query.limit(load_limit_);
      _uf.serialize(id_query);

      Database::InsertQuery tmp_table_query = Database::InsertQuery(getTempTableName(),
          id_query);
      LOGGER.debug(boost::format("temporary table '%1%' generated sql = %2%")
          % getTempTableName() % tmp_table_query.str());

      Database::SelectQuery object_info_query;
      object_info_query.select() << "t_1.id, t_1.zone_id, t_2.fqdn, "
                                 << "t_1.crdate::timestamptz AT TIME ZONE 'Europe/Prague', "
                                 << "t_1.taxdate, t_5.fromdate, t_5.todate, t_4.typ, t_1.prefix, "
                                 << "t_1.registrar_id, t_1.balance, t_1.operations_price, "
                                 << "t_1.vat, t_1.total, t_1.totalvat, "
                                 << "t_1.file, t_1.fileXML, t_3.organization, t_3.street1, "
                                 << "t_3.city, t_3.postalcode, "
                                 << "TRIM(t_3.ico), TRIM(t_3.dic), TRIM(t_3.varsymb), "
                                 << "t_3.handle, t_3.vat, t_3.id, t_3.country, "
                                 << "t_6.name as file_name, t_7.name as filexml_name";

      object_info_query.from() << "tmp_invoice_filter_result tmp "
                               << "JOIN invoice t_1 ON (tmp.id = t_1.id) "
                               << "JOIN zone t_2 ON (t_1.zone_id = t_2.id) "
                               << "JOIN registrar t_3 ON (t_1.registrar_id = t_3.id) "
                               << "JOIN invoice_prefix t_4 ON (t_4.id = t_1.invoice_prefix_id) "
                               << "LEFT JOIN invoice_generation t_5 ON (t_1.id = t_5.invoice_id) "
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
          Money    credit    = std::string(*(++col));
          Money    price     = std::string(*(++col));
          Decimal    vat_rate = std::string( *(++col));
          Money    total     = std::string(*(++col));
          Money    total_vat = std::string(*(++col));
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
        
        LOGGER.debug(boost::format("list of invoices size: %1%") % data_.size());
        
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
        source_query.select() << "tmp.id, icm.credit, sri.vat, sri.prefix, "
                              << "icm.balance, sri.id, sri.total, "
                              << "sri.totalvat, sri.crdate";
        source_query.from() << "tmp_invoice_filter_result tmp "
                            << "JOIN invoice_credit_payment_map icm ON (tmp.id = icm.ac_invoice_id) "
                            << "JOIN invoice sri ON (icm.ad_invoice_id = sri.id) ";
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
          action_query.select() << "tmp.id, SUM(icm.price), i.vat, o.name, "
                                << "io.crdate::timestamptz AT TIME ZONE 'Europe/Prague', "
                                << "io.date_from, io.date_to, io.operation_id, io.quantity, "
                                << "CASE "
                                << "  WHEN io.quantity = 0 THEN 0 "
                                << "  ELSE SUM(icm.price) / io.quantity END, "
                                << "o.id";
          action_query.from() << "tmp_invoice_filter_result tmp "
                              << "JOIN invoice_operation io ON (tmp.id = io.ac_invoice_id) "
                              << "JOIN invoice_operation_charge_map icm ON (io.id = icm.invoice_operation_id) "
                              << "JOIN invoice i ON (icm.invoice_id = i.id) "
                              << "LEFT JOIN object_registry o ON (io.object_id = o.id) ";
          action_query.group_by() << "tmp.id, o.name, io.crdate, io.date_from, io.date_to, "
                                  << "io.operation_id, io.quantity, o.id, i.vat";
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
                LOGGER.info("Statement timeout in request list.");
                clear();
                throw;
            } else {
                LOGGER.error(boost::format("%1%") % ex.what());
                clear();
            }
        }
      catch (std::exception& ex) {
        LOGGER.error(boost::format("%1%") % ex.what());
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
                where << "AND " << "i.registrar_id" << "= $" << sql_params.size() << "::bigint ";
            }
            if (zoneFilter)
            {
                sql_params.push_back(boost::lexical_cast<std::string>(zoneFilter));
                where << "AND " << "i.zone_id" << "= $" << sql_params.size() << "::bigint ";
            }
            if (typeFilter && typeFilter<=2)
            {
                from << ", invoice_prefix ip ";
                where << "AND i.invoice_prefix_id=ip.id ";
                sql_params.push_back(boost::lexical_cast<std::string>(typeFilter-1));
                where << "AND " << "ip.typ" << "=$" << sql_params.size() << "::integer ";
            }
            if (!varSymbolFilter.empty() || !registrarHandleFilter.empty())
            {
                from << ", registrar r ";
                where << "AND i.registrar_id=r.id ";
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
                    << "(i.prefix)::text" << " ILIKE TRANSLATE($" << sql_params.size() << "::text,'*?','%_') ";
            }
            if (!crDateFilter.begin().is_special())
            {
                sql_params.push_back(boost::lexical_cast<std::string>(
                        to_iso_extended_string(crDateFilter.begin().date())));
                 where << "AND " << "i.crdate" << ">=$"
                   <<  sql_params.size() << "::timestamp ";
            }
            if (!crDateFilter.end().is_special())
            {
                sql_params.push_back(
                    to_iso_extended_string(crDateFilter.end().date() + boost::gregorian::days(1))
                    );
                where << "AND " << "i.crdate" << " < $"
                        <<  sql_params.size() << "::timestamp ";
            }
            if ((!taxDateFilter.begin().is_special()))
            {
                sql_params.push_back(boost::lexical_cast<std::string>(
                    to_iso_extended_string(taxDateFilter.begin())));
               where << "AND " << "i.taxdate" << ">=$"
                 <<  sql_params.size() << "::timestamp ";
            }
            if ((!taxDateFilter.end().is_special()))
            {
                sql_params.push_back(
                    to_iso_extended_string(taxDateFilter.end().date() + boost::gregorian::days(1)));
                where << "AND " << "i.taxdate" << " < $"
                    << sql_params.size() << "::timestamp ";
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
                from << ", invoice_operation io ";
                where << "AND i.id=io.ac_invoice_id ";
                sql_params.push_back(boost::lexical_cast<std::string>(objectIdFilter));
                where << "AND " << "io.object_id" << "=$" << sql_params.size() << "::bigint ";
            }
            if (!objectNameFilter.empty())
            {
                from << ", invoice_operation ioh, object_registry obr ";
                where << "AND i.id=ioh.ac_invoice_id AND obr.id=ioh.object_id ";
                sql_params.push_back(boost::lexical_cast<std::string>(objectNameFilter));
                where << "AND "
                       << "obr.name" << " ILIKE TRANSLATE($" << sql_params.size() << "::text,'*?','%_') ";
            }

            if (!advanceNumberFilter.empty())
            {
                from << ", invoice_operation io2 "
                << ", invoice_operation_charge_map iocm "
                << ", invoice advi ";
                where << "AND i.id=io2.ac_invoice_id "
                << "AND iocm.invoice_operation_id=io2.id AND iocm.invoice_id=advi.id ";
                sql_params.push_back(boost::lexical_cast<std::string>(advanceNumberFilter));
                where << "AND "
                       << "(advi.prefix)::text" << " ILIKE TRANSLATE($" << sql_params.size() << "::text,'*?','%_') ";
            }
            // complete sql end do the query
            sql << from.rdbuf() << where.rdbuf();

            Database::Result res1 = conn.exec_params(sql.str(), sql_params);
            Database::Result res2 = conn.exec("ANALYZE tmp_invoice_filter_result");
            // initialize list of invoices using temporary table

            Database::Result res3 = conn.exec(
                  "SELECT "
                  " i.id, i.zone_id, i.crdate::timestamptz AT TIME ZONE 'Europe/Prague',"
                  " i.taxdate, ig.fromdate, "
                  " ig.todate, ip.typ, i.prefix, i.registrar_id, i.balance, "
                  " i.operations_price, i.vat, i.total, i.totalvat, "
                  " i.file, i.fileXML, "
                  " r.organization, r.street1, "
                  " r.city, r.postalcode, TRIM(r.ico), TRIM(r.dic), TRIM(r.varsymb), "
                  " r.handle, r.vat, r.id, z.fqdn, r.country "
                  "FROM "
                  " tmp_invoice_filter_result it "
                  " JOIN invoice i ON (it.id=i.id) "
                  " JOIN zone z ON (i.zone_id=z.id) "
                  " JOIN registrar r ON (i.registrar_id=r.id) "
                  " JOIN invoice_prefix ip ON (ip.id=i.invoice_prefix_id) "
                  " LEFT JOIN invoice_generation ig ON (i.id=ig.invoice_id) "
                  // temporary static sorting
                  " ORDER BY it.id"
              );

            for (unsigned i=0; i < res3.size(); ++i)
            {
                LOGGER.debug(
                    boost::format(
                    "res3 i.id %1% i.zone_id %2% i.typ %3% i.prefix %4%"
                    " i.registrar_id %5% i.balance %6% i.operations_price %7% i.vat %8%"
                    " i.total %9% i.totalvat %10%")
                    % std::string(res3[i][0])
                    % std::string(res3[i][1])
                    % (int(res3[i][6]) == 0 ? IT_DEPOSIT : IT_ACCOUNT)
                    % std::string(res3[i][7])
                    % std::string(res3[i][8])
                    % std::string(res3[i][9])
                    % std::string(res3[i][10])
                    % std::string(res3[i][11])
                    % std::string(res3[i][12])
                    % std::string(res3[i][13])
                );

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

                Money    credit         = std::string(res3[i][9]);

                Money    price          = std::string(res3[i][10]);

                std::string        vat_rate       = res3[i][11];

                Money    total          = std::string(res3[i][12]);

                Money    total_vat      = std::string(res3[i][13]);

                Database::ID       filePDF        = res3[i][14];
                Database::ID       fileXML        = res3[i][15];

                std::string        c_organization = res3[i][16];
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
                                                Decimal(vat_rate),
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
              action_query.select() << "tmp.id, SUM(icm.price), i.vat, o.name, "
                                    << "io.crdate::timestamptz AT TIME ZONE 'Europe/Prague', "
                                    << "io.date_from, io.date_to, io.operation_id, io.quantity, "
                                    << "CASE "
                                    << "  WHEN io.quantity = 0 THEN 0 "
                                    << "  ELSE SUM(icm.price) / io.quantity END, "
                                    << "o.id";
              action_query.from() << "tmp_invoice_filter_result tmp "
                                  << "JOIN invoice_operation io ON (tmp.id = io.ac_invoice_id) "
                                  << "JOIN invoice_operation_charge_map icm ON (io.id = icm.invoice_operation_id) "
                                  << "JOIN invoice i ON (icm.invoice_id = i.id) "
                                  << "LEFT JOIN object_registry o ON (io.object_id = o.id) ";
              action_query.group_by() << "tmp.id, o.name, io.crdate, io.date_from, io.date_to, "
                                      << "io.operation_id, io.quantity, o.id, i.vat";
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
                source_query.select() << "tmp.id, icm.credit, sri.vat, sri.prefix, "
                                      << "icm.balance, sri.id, sri.total, "
                                      << "sri.totalvat, sri.crdate";
                source_query.from() << "tmp_invoice_filter_result tmp "
                                    << "JOIN invoice_credit_payment_map icm ON (tmp.id = icm.ac_invoice_id) "
                                    << "JOIN invoice sri ON (icm.ad_invoice_id = sri.id) ";
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
              LOGGER.error("timeout");
              clear();
            }
            else
            {
              LOGGER.error(boost::format("%1%") % ex.what());
              clear();
            }
            throw;
        }
        catch (std::exception& ex)
        {
            LOGGER.error(boost::format("%1%") % ex.what());
            clear();
            throw;
        }

    }//reload()

    
    /// export all invoices on the list using given exporter
    InvoiceIdVect doExport(Exporter *_exporter) {
        if(!_exporter) {
            LOGGER.error("Exporter::doExport _exporter");
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
            LOGGER.error("Exporter::doExport dynamic_cast<InvoiceImpl*> failed");
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
    void send() { //throw (SQL_ERROR)
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
            LOGGER.error(
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
              LOGGER.error(
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
        << "LEFT JOIN invoice_generation g ON (g.invoice_id=i.id) "
        << "LEFT JOIN invoice_mails im ON (im.invoiceid=i.id) "
        << "LEFT JOIN zone z ON (z.id = i.zone_id) "
        << "WHERE i.registrar_id=r.id "
        << "AND im.mailid ISNULL "
        << "AND NOT(r.email ISNULL OR TRIM(r.email)='')"
        << "UNION "
        << "SELECT r.email, g.fromdate, g.todate, NULL, NULL, g.id, "
        << "NULL, z.fqdn "
        << "FROM registrar r, invoice_generation g "
        << "LEFT JOIN invoice_mails im ON (im.genid=g.id) "
        << "LEFT JOIN zone z ON (z.id = g.zone_id) "
        << "WHERE g.registrar_id=r.id AND g.invoice_id ISNULL "
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
      Database::Result res = conn.exec("SELECT vat, koef, valid_to FROM price_vat");
      for (unsigned i=0; i < res.size(); ++i) {
        vatList.push_back(
            VAT(Decimal(std::string(res[i][0])),
                    Decimal(std::string(res[i][1])),
                (date(res[i][2].isnull()? date(not_a_date_time) : from_string(res[i][2])))
                )
        );
      }//for res
    }//if empty
  }

  Money ManagerImpl::countVAT(Money price_without_vat, Decimal vatRate, bool use_coef) {
    Money vat;
    if (!use_coef) {
        vat = price_without_vat * (vatRate / Decimal("100"));
        LOGGER.debug(
            std::string("ManagerImpl::countVAT: ")+ vat.get_string()
            + " price: " + price_without_vat.get_string()+ " vatRate: " + vatRate.get_string()
            + " base: true use_coef: false");
    }
    else {
        const VAT *v = getVAT(vatRate);
        Decimal coef = v ? v->koef : Decimal("0");
        vat = (price_without_vat * coef / (Decimal("1") - coef)).round_half_up(2);
        LOGGER.debug(
            std::string("ManagerImpl::countVAT: ")+ vat.get_string()
            + " price: " + price_without_vat.get_string()+ " vatRate: " + vatRate.get_string()
            + " base: true use_coef: true coef: " + coef.get_string());
    }
    return vat;
  }
  
  const VAT * ManagerImpl::getVAT(Decimal rate) {
    // late initialization would brake constness
    this->initVATList();
    std::vector<VAT>::const_iterator ci = find(
        vatList.begin(),vatList.end(),rate
    );
    return ci == vatList.end() ? NULL : &(*ci);
  }
  
  InvoiceIdVect ManagerImpl::archiveInvoices(bool send
          , InvoiceIdVect archive_only_this_if_set) {
      
      if(docman == NULL || mailman == NULL) {
        LOGGER.error("archiveInvoices: No docman or mailman specified in c-tor. ");
        throw std::runtime_error("archiveInvoices: No docman or mailman specified in c-tor");
      }
      
      InvoiceIdVect ret_inv;

    try {
      // archive unarchived invoices
      ExporterArchiver exporter(docman);
      ListImpl l(this);
      l.setArchivedFilter(ListImpl::AF_UNSET);
/* TODO
      for(InvoiceIdVect::const_iterator it = archive_only_this_if_set.begin()
              ; it != archive_only_this_if_set.end() ; ++it )
      {
          l.setIdFilter(*it);//archive only set id
      }
*/
      l.reload();
      ret_inv = l.doExport(&exporter);
      if (send) {
        Mails m(mailman);
        m.load();
        m.send();
      }
    }//try
    catch (const std::exception& ex) {
      LOGGER.error(std::string("Exception in archiveInvoices: ") + ex.what());
      throw;
    }

    catch (...) {
      LOGGER.error("Exception in archiveInvoices.");
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

      Database::Result res = conn.exec_params(
              "SELECT credit FROM registrar_credit rc"
              " JOIN registrar r ON (rc.registrar_id = r.id)"
              " WHERE rc.zone_id = $1::bigint AND r.handle = $2::text",
              Database::query_param_list(zone)(registrarHandle));

      std::string result = "0.00";
      if (res.size() == 1) {
          result = std::string(res[0][0]);
      }

      LOGGER.debug(std::string("Invoice::ManagerImpl::getCreditByZone():")
            + " registrar=" + registrarHandle
            + " zone_id=" + boost::lexical_cast<std::string>(zone)
            + " credit=" + result);

      return result;
  }

  bool ManagerImpl::insertInvoicePrefix(unsigned long long zoneId,
          int type, int year, unsigned long long prefix) 
  {
      TRACE("Invoicing::Manager::insertInvoicePrefix(...)");
      Database::Connection conn = Database::Manager::acquire();
      std::stringstream query;
      query << "INSERT INTO invoice_prefix (zone_id, typ, year, prefix) VALUES"
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
  
  ///create next year invoice prefixes for zones and invoice types in invoice_number_prefix if they don't exist
  void ManagerImpl::createInvoicePrefixes(bool for_current_year)
  {
      TRACE("Invoicing::Manager::createInvoicePrefixes()");
      Database::Connection conn = Database::Manager::acquire();
      Database::Transaction tx(conn);
      boost::gregorian::date local_today = boost::gregorian::day_clock::local_day();
      int next_year = local_today.year() + (for_current_year ? 0 : 1);

      //invoice type 0-advance 1-account ...

      //get existing prefixes
      Database::Result exist_res = conn.exec_params(
          "select z.fqdn, it.name, ip.year, ip.prefix "
          " from invoice_prefix ip "
          " join zone z on ip.zone_id=z.id "
          " join invoice_type it on ip.typ = it.id "
          " join invoice_number_prefix inp "
          "  on inp.zone_id = z.id and inp.invoice_type_id = it.id "
          " where ip.year = $1::integer "
          , Database::query_param_list (next_year));

      //print existing prefixes to stderr
      if(exist_res.size() > 0)
      {
          std::cerr << "already existing prefixes for next year: " << next_year <<std::endl;
          std::cerr << "zone_fqdn | typ | year | prefix"  <<std::endl;
      }
      for(unsigned i = 0 ; i < exist_res.size(); ++i)
      {
          std::cerr
              << std::string(exist_res [i][0]) << " | "
              << std::string(exist_res [i][1]) << " | "
              << std::string(exist_res [i][2]) << " | "
              << std::string(exist_res [i][3])
          << std::endl;
      }

      //get nonexisting prefixes
      if(exist_res.size() > 0)
      {
          Database::Result add_res = conn.exec_params(
              " select zone_fqdn, typ, year, prefix from "
              //--2 add
              " ( "
              " select z.fqdn as zone_fqdn, it.name as typ, $1::integer as year "
              " , inp.prefix::numeric * 10000000 + mod($1::integer , 100) * 100000 + 1 as prefix "
              " from zone z "
              " join invoice_number_prefix inp on inp.zone_id = z.id "
              " join invoice_type it on it.id = inp.invoice_type_id "
              " EXCEPT "
              //--those already there
              " select z.fqdn as zone_fqdn, it.name as typ, ip.year as year "
              " , inp.prefix::numeric * 10000000 + mod($1::integer , 100) * 100000 + 1 as prefix "
              " from invoice_prefix ip "
              " join zone z on ip.zone_id=z.id "
              " join invoice_type it on it.id=ip.typ "
              " join invoice_number_prefix inp on inp.zone_id = z.id and inp.invoice_type_id = it.id "
              " where ip.year = $1::integer "
              " ) as add_pfx "
              , Database::query_param_list (next_year));

          //print existing prefixes to stderr
          std::cerr << "\nto add " << add_res.size() <<" prefixes for next year: " << next_year <<std::endl;
          if(add_res.size() > 0) std::cerr << "zone_fqdn | typ | year | prefix"  <<std::endl;

          for(unsigned i = 0 ; i < add_res.size(); ++i)
          {
              std::cerr
                  << std::string(add_res [i][0]) << " | "
                  << std::string(add_res [i][1]) << " | "
                  << std::string(add_res [i][2]) << " | "
                  << std::string(add_res [i][3])
              << std::endl;
          }

      }//if prefix exist

      //insert new prefixes
      conn.exec_params(
          "insert into invoice_prefix(zone_id, typ, year, prefix) "
          " select zone_id, typ, year, prefix from "
          //--2 add
          " ( "
          " select z.id as zone_id, it.id as typ, $1::integer as year "
          " , inp.prefix::numeric * 10000000 + mod($1::integer , 100) * 100000 + 1 as prefix "
          " from zone z "
          " join invoice_number_prefix inp on inp.zone_id = z.id "
          " join invoice_type it on it.id = inp.invoice_type_id "
          " EXCEPT "
          //--those already there
          " select z.id as zone_id, ip.typ as typ, ip.year as year "
          " , inp.prefix::numeric * 10000000 + mod($1::integer , 100) * 100000 + 1 as prefix "
          " from invoice_prefix ip "
          " join zone z on ip.zone_id=z.id "
          " join invoice_type it on it.id=ip.typ "
          " join invoice_number_prefix inp on inp.zone_id = z.id and inp.invoice_type_id = it.id "
          " where ip.year = $1::integer "
          " ) as add_pfx "
          , Database::query_param_list (next_year));

      tx.commit();

  }
/// add invoice number prefix for zone and invoice type
  void ManagerImpl::addInvoiceNumberPrefix( unsigned long prefix
          , const std::string& zone_fqdn
          , const std::string invoice_type_name)
  {
      TRACE("Invoicing::Manager::addInvoiceNumberPrefix");
      Database::Connection conn = Database::Manager::acquire();
      Database::Transaction tx(conn);

      conn.exec_params(
          "insert into invoice_number_prefix( prefix, zone_id, invoice_type_id) "
          " values ($1::integer , (select id from zone where fqdn=$2::text) "
          " , (select id from invoice_type where name=$3::text))"
          , Database::query_param_list (prefix)(zone_fqdn)(invoice_type_name));
      tx.commit();
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

// REWRITTEN AS PART OF #18734 POLL REWRITE, TO DELETE?
 std::string getRequestUnitPrice(unsigned zone_id)
 {
     Database::Connection conn = Database::Manager::acquire();

     // get per request price
     Database::Result res_price = conn.exec_params(
              "SELECT price"
              " FROM price_list pl"
              " WHERE pl.zone_id=$1::integer"
              " AND valid_from < 'now()'"
              " AND ( valid_to IS NULL OR valid_to > 'now()')"
              " AND operation_id=$2::integer"
              " ORDER BY valid_from DESC"
              " LIMIT 1",
              Database::query_param_list
                   (zone_id)
                   (static_cast<int>(LibFred::Invoicing::INVOICING_GeneralOperation))
     );

     if(res_price.size() != 1 || res_price[0][0].isnull()) {
         throw std::runtime_error("Entry for request fee not found in price_list");
     }

     return std::string(res_price[0][0]);
 }

// PARTIALLY REWRITTEN AS PART OF #18734 POLL REWRITE
 void getRequestFeeParams(unsigned *zone_id, unsigned *base_free_count, unsigned *per_domain_free_count)
 {
     Database::Connection conn = Database::Manager::acquire();

     // get reuest fee parametres
     Database::Result res_params = conn.exec(
               "SELECT zone_id, count_free_base, count_free_per_domain"
               " FROM request_fee_parameter"
               " WHERE valid_from < now()"
               " ORDER BY valid_from DESC"
               " LIMIT 1");

     if(res_params.size() != 1
             || res_params[0][0].isnull()
             || res_params[0][1].isnull()
             || res_params[0][2].isnull()) {
         throw std::runtime_error("Couldn't find a valid record in request_fee_parameter table");
     }

     if(zone_id != NULL) {
         *zone_id = res_params[0][0];
     }
     if(base_free_count != NULL) {
         *base_free_count = res_params[0][1];
     }
     if(per_domain_free_count != NULL) {
         *per_domain_free_count = res_params[0][2];
     }

 }

  
}; // Invoicing
}; // Fred
