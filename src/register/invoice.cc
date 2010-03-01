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
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/checked_delete.hpp>

#include "common_impl.h"
#include "invoice.h"
#include "log/logger.h"
#include "types/convert_sql_db_types.h"
#include "types/sqlize.h"
#include "old_utils/dbsql.h"

#include "documents.h"
#include "sql.h"

#include "log/logger.h"

using namespace boost::gregorian;
using namespace boost::posix_time;

class SQL_ERROR;

#ifdef MAKE_TIME_DEF
#undef MAKE_TIME_DEF
#endif

#ifdef MAKE_DATE_DEF
#undef MAKE_DATE_DEF
#endif

#define MAKE_TIME_DEF(ROW,COL,DEF)  \
  (ptime(db->IsNotNull(ROW,COL) ? \
   time_from_string(db->GetFieldValue(ROW,COL)) : DEF))
#define MAKE_TIME(ROW,COL) \
  MAKE_TIME_DEF(ROW,COL,ptime(not_a_date_time))


/*
#define MAKE_TIME_DEF(ROW,COL,DEF)  \
  (ptime(!res[ROW][COL].isnull() ? \
   time_from_string(res[ROW][COL]) : DEF))
#define MAKE_TIME(ROW,COL) \
  MAKE_TIME_DEF(ROW,COL,ptime(not_a_date_time))
#define MAKE_TIME_NEG(ROW,COL) \
  MAKE_TIME_DEF(ROW,COL,ptime(neg_infin))
#define MAKE_TIME_POS(ROW,COL) \
  MAKE_TIME_DEF(ROW,COL,ptime(pos_infin))
 **/

/*
#define MAKE_TIME_DEF(ROW,COL,DEF)  \
  (ptime(db->IsNotNull(ROW,COL) ? \
   time_from_string(db->GetFieldValue(ROW,COL)) : DEF))
#define MAKE_DATE_DEF(ROW,COL,DEF)  \
 (date(db->IsNotNull(ROW,COL) ? from_string(db->GetFieldValue(ROW,COL)) : DEF))
 **/


#define MAKE_DATE_DEF(ROW,COL,DEF)  \
 (date(db->IsNotNull(ROW,COL) ? from_string(db->GetFieldValue(ROW,COL)) : DEF))
#define MAKE_DATE(ROW,COL)  \
  MAKE_DATE_DEF(ROW,COL,date(not_a_date_time))
#define MAKE_DATE_NEG(ROW,COL) \
  MAKE_DATE_DEF(ROW,COL,date(neg_infin))
#define MAKE_DATE_POS(ROW,COL) \
  MAKE_DATE_DEF(ROW,COL,date(pos_infin))


class autoDB : public DB {
public:
    autoDB() : DB() {};
    
    ~autoDB() {
        Disconnect();
    }
};


#define STR_TO_MONEY(x) atol(x)

namespace Register {
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
  const VAT *getVAT(unsigned rate) const;
  /// find unarchived invoices. archive then and send them by email
  void archiveInvoices(bool send) const;
  /// create empty list of invoices      
  virtual List* createList() const;
  /// return credit for registrar by zone
  virtual Money
      getCreditByZone(const std::string& registrarHandle, TID zone);
  virtual bool insertInvoicePrefix(unsigned long long zoneId,
          int type, int year, unsigned long long prefix);
  virtual bool insertInvoicePrefix(const std::string &zoneName,
          int type, int year, unsigned long long prefix);

  /** this is now usable only for create domain
   */
  virtual bool domainBilling(
            DB *db, 
            const Database::ID &zone,
            const Database::ID &registrar,
            const Database::ID &objectId,
            const Database::Date &exDate,
            const int &units_count,
            bool renew) {
      TRACE("[CALL] Register::Invoicing::Manager::domainBilling()");

      if(!renew) {
          return db->BillingCreateDomain(registrar, zone, objectId);
      } else {
          return db->BillingRenewDomain(registrar, zone, objectId, units_count, exDate.to_string().c_str());
      }
  };

    /**
     *returns 0 in case of failure, id of invoice otherwise
     * type is int just because MakeNewInoviceAdvance returns it.
     */
int createDepositInvoice(Database::Date date, int zoneId, int registrarId, long price) {

    /* HACK! we need to use same transaction */
    Database::Connection conn = Database::Manager::acquire();
    DB db(conn);
    // if(!db.OpenDatabase(Database::Manager::getConnectionString())) {
    //     LOGGER(PACKAGE).error(" autoDB: Failed to open the database. ");
    //     return 0;
    // }

    // TODO conversion with loss of precision
    int ret = db.MakeNewInvoiceAdvance(date.to_string().c_str(), zoneId, registrarId, price);

    switch(ret)  {
        case -1:
                throw std::runtime_error(" Insert into table invoice has failed. ");
                break;
        case -2: 
                throw std::runtime_error(" Couldn't find invoice prefix. ");
                break;
        case -3:
                throw std::runtime_error(" Select from registrar table failed. ");
                break;
        case -4:
                throw std::runtime_error(" Number of selected rows is not 1 as it should be.");
                break;
        default:
                // this shouldn't happen
                break;
    }
    return ret;
}


bool factoring_all(const char *database, const char *zone_fqdn, const char *taxdateStr, const char *todateStr)
{
  autoDB db;
  int *regID = 0;
  int i, num =-1;
  char timestampStr[32];
  int invoiceID = 0;
  int zone;
  int ret = 0;

  if (db.OpenDatabase(database) ) {

      LOGGER(PACKAGE).debug ( boost::format("successfully  connect to DATABASE %1%") % database);

    if (db.BeginTransaction() ) {
      get_timestamp(timestampStr, get_utctime_from_localdate(todateStr) );

      if ( (zone = db.GetNumericFromTable("zone", "id", "fqdn", zone_fqdn) )) {
        std::stringstream sql;
        sql
            << "SELECT r.id FROM registrar r, registrarinvoice i WHERE r.id=i.registrarid "
            << "AND r.system=false AND i.zone=" << zone
            << " AND i.fromdate<=CURRENT_DATE";
        if (db.ExecSelect(sql.str().c_str()) && db.GetSelectRows() > 0) {
          num = db.GetSelectRows();
          regID= new int[num];
          for (i = 0; i < num; i ++) {
            regID[i] = atoi(db.GetFieldValue(i, 0) );
          }
          db.FreeSelect();

          if (num > 0) {
            for (i = 0; i < num; i ++) {
              invoiceID = db.MakeFactoring(regID[i], zone, timestampStr,
                taxdateStr);
              LOGGER(PACKAGE).notice(boost::format("Vygenerovana fa %1% pro regID %2% ") % invoiceID % regID[i] );

              if (invoiceID >=0)
                ret = CMD_OK;
              else {
                ret = 0;
                break;
              }

            }
          }
          delete[] regID;
        }
      } else {
        LOGGER(PACKAGE).error( boost::format("unkown zone %1% \n") % zone_fqdn );
      }

      db.QuitTransaction(ret);
    }

    // db.Disconnect(); not needed anymore
  }

  if (ret)
    return invoiceID;
  else
    return -1; // err
}

// close invoice to registar handle for zone make taxDate to the todateStr
int factoring(const char *database, const char *registrarHandle, const char *zone_fqdn, const char *taxdateStr, const char *todateStr)
{
  autoDB db;
  int regID;
  char timestampStr[32];
  int invoiceID = -1;
  int zone;
  int ret = 0;

  if (db.OpenDatabase(database) ) {

    LOGGER(PACKAGE).debug ( boost::format("successfully  connect to DATABASE %1%") % database);

    if (db.BeginTransaction() ) {

      if ( (regID = db.GetRegistrarID(  registrarHandle ) )) {
        if ( (zone = db.GetNumericFromTable("zone", "id", "fqdn", zone_fqdn) )) {

          get_timestamp(timestampStr, get_utctime_from_localdate(todateStr) );
          // make invoice
          invoiceID = db.MakeFactoring(regID, zone, timestampStr, taxdateStr);

        } else {
          LOGGER(PACKAGE).error( boost::format("unknown zone %1% \n") % zone_fqdn );
        }
      } else {
        LOGGER(PACKAGE).error( boost::format("unknown registrarHandle %1% ") % registrarHandle );
      }

      if (invoiceID >=0)
        ret = CMD_OK; // OK succesfully invocing

      db.QuitTransaction(ret);
    }

    // db.Disconnect(); not needed anymore
  }

  if (ret)
    return invoiceID;
  else
    return -1; // err
}

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
    
  PaymentSourceImpl(DB *db, unsigned l, ManagerImpl *man) :
    PaymentImpl(
    STR_TO_MONEY(db->GetFieldValue(l,2)),
    STR_TO_ID(db->GetFieldValue(l,6)),
    man->countVAT(
        STR_TO_MONEY(db->GetFieldValue(l,2)),
        STR_TO_ID(db->GetFieldValue(l,6)),
        true
    )), number(atoll(db->GetFieldValue(l, 1))), credit(STR_TO_MONEY(db->GetFieldValue(l,3))), id(STR_TO_ID(db->GetFieldValue(l,4))), totalPrice(STR_TO_MONEY(db->GetFieldValue(l,5))), totalVat(STR_TO_MONEY(db->GetFieldValue(l,7))), crtime(MAKE_TIME(l,8)) {
  }
     
  
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

    // TODO db shouldn't be saved anywhere in the dtor because caller reload()
    // doesnt count on it and it retains the ownership of the db resource
  PaymentActionImpl(DB *db, unsigned l, ManagerImpl *man) :
    PaymentImpl(
    STR_TO_MONEY(db->GetFieldValue(l,7)),
    STR_TO_ID(db->GetFieldValue(l,9)),
    man->countVAT(
        STR_TO_MONEY(db->GetFieldValue(l,7)),
        STR_TO_ID(db->GetFieldValue(l,9)),
        true
    )), 
        objectName(db->GetFieldValue(l, 1)),
        actionTime(MAKE_TIME(l,2)),
        exDate(MAKE_DATE(l,3)),
        action(atoi(db->GetFieldValue(l, 4)) == 1 ? PAT_CREATE_DOMAIN
                                                     : PAT_RENEW_DOMAIN),
    unitsCount(atoi(db->GetFieldValue(l, 5))),
        pricePerUnit(STR_TO_MONEY(db->GetFieldValue(l,6))),
        objectId(STR_TO_ID(db->GetFieldValue(l,8))) {
  }
    
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


class InvoiceImpl : public Register::CommonObjectImpl, 
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
  /// initialize invoice from result set=db with row=l

    
  InvoiceImpl(DB *db, ManagerImpl *_man, unsigned l) :
      CommonObjectImpl(STR_TO_ID(db->GetFieldValue(l,0))),
      // this class should not store pointer to DB
      dbc(NULL),  
      zone(STR_TO_ID(db->GetFieldValue(l,1))), 
      zoneName(db->GetFieldValue(l, 26)), 
      crTime(MAKE_TIME(l,2)), 
      taxDate(MAKE_DATE(l,3)), 
      accountPeriod(MAKE_DATE_NEG(l,4),MAKE_DATE_POS(l,5)), 
      type(atoi(db->GetFieldValue(l, 6)) == 0 ? IT_DEPOSIT : IT_ACCOUNT),
      number(atoll(db->GetFieldValue(l, 7))), 
      registrar(STR_TO_ID(db->GetFieldValue(l,8))),
      credit(STR_TO_MONEY(db->GetFieldValue(l,9))), 
      price(STR_TO_MONEY(db->GetFieldValue(l,10))), 
      vatRate(atoi(db->GetFieldValue(l, 11))),
      total(STR_TO_MONEY(db->GetFieldValue(l,12))), 
      totalVAT(STR_TO_MONEY(db->GetFieldValue(l,13))), 
      filePDF(STR_TO_ID(db->GetFieldValue(l,14))), 
      fileXML(STR_TO_ID(db->GetFieldValue(l,15))), 
      varSymbol(db->GetFieldValue(l, 22)), 
      client(STR_TO_ID(db->GetFieldValue(l,25)),
             db->GetFieldValue(l,23),
             db->GetFieldValue(l,16),
             "", // fullname is empty
             db->GetFieldValue(l,17),
             db->GetFieldValue(l,18),
             db->GetFieldValue(l,19),
             db->GetFieldValue(l,27),
             db->GetFieldValue(l,20),
             db->GetFieldValue(l,21),
             "", // registration is empty
             "", // reclamation is empty
             "", // url is empty
             "", // email is empty
             "", // phone is empty
             "", // fax is empty
             db->GetFieldValue(l,24)[0] == 't'), 
      storeFileFlag(false), 
      ap(_man), 
      man(_man) {
  }

     
  
  InvoiceImpl(TID _id, TID _zone, std::string& _zoneName, ptime _crTime, date _taxDate,
              date_period& _accountPeriod, Type _type, unsigned long long _number,
              TID _registrar, Money _credit, Money _price, short _vatRate, Money _total,
              Money _totalVAT, TID _filePDF, TID _fileXML, std::string& _varSymbol,
              SubjectImpl& _client, const std::string &_filepdf_name, const std::string &_filexml_name,
              ManagerImpl *_manager) : CommonObjectImpl(_id),
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
  void storeFile()  {
    // cannot rollback generated files so ignoring if XML file hasn't
    // been generated
    autoDB dbc;
    if(!dbc.OpenDatabase(Database::Manager::getConnectionString())) {
        LOGGER(PACKAGE).error(" autoDB: Failed to open the database. ");
        throw SQL_ERROR();
    }

    if (storeFileFlag && filePDF) {
      std::stringstream sql;
      sql << "UPDATE invoice SET file=" << filePDF;
      if (fileXML) {
        sql << ",fileXML=" << fileXML;
      };
      sql << " WHERE id=" << getId();      
      if (!dbc.ExecSQL(sql.str().c_str()))
        throw SQL_ERROR();
    }
  }
  /// export invoice using given exporter
  void doExport(Exporter *exp) {
    exp->doExport(this);
  }
  /// initialize list of actions from sql result

  // ownership of db remains in the caller
  void addAction(DB *db, unsigned row) {
    actions.push_back(new PaymentActionImpl(db,row,man));
    ap.addAction(actions.back()); // update partitioning
  }
   
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

  
  void addSource(DB *db, unsigned row) {
    PaymentSourceImpl *ps = new PaymentSourceImpl(db,row,man);
    sources.push_back(ps);
    // init vat groups, if vat rate exists, add it, otherwise create new
    std::vector<PaymentImpl>::iterator i = find(paid.begin(),
                                               paid.end(),
                                               ps->getVatRate() );
    if (i != paid.end())
      i->add(ps);
    else
      paid.push_back(PaymentImpl(ps));
  }  

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
    virtual const int getVat() const {
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
  std::ostream& doExport(const Subject* s) {
    out << TAG(id,s->getId()) << TAG(name,s->getName()) << TAG(fullname,s->getFullname()) << TAGSTART(address) << TAG(street,s->getStreet()) << TAG(city,s->getCity()) << TAG(zip,s->getZip()) << TAG(country,s->getCountry()) << TAGEND(address) << TAG(ico,s->getICO()) << TAG(vat_number,s->getVatNumber()) << TAG(registration,s->getRegistration()) << TAG(reclamation,s->getReclamation()) << TAG(url,s->getURL()) << TAG(email,s->getEmail()) << TAG(phone,s->getPhone()) << TAG(fax,s->getFax()) << TAG(vat_not_apply,(s->getVatApply() ? 0 : 1)); return out;} 
    virtual void doExport(Invoice *i)
    {
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
          // TODO log something more specific
        LOGGER(PACKAGE).error("Exception in ExporterArchiver::doExport.");
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
  class ListImpl : public Register::CommonListImpl,
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
        source_query.select() << "tmp.id, ipm.credit * 100, sri.vat, sri.prefix, "
                              << "ipm.balance * 100, sri.id, sri.total * 100, "
                              << "sri.totalvat * 100, sri.crdate";
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
          action_query.select() << "tmp.id, SUM(ipm.price) * 100, i.vat, o.name, "
                                << "ior.crdate::timestamptz AT TIME ZONE 'Europe/Prague', "
                                << "ior.exdate, ior.operation, ior.period, "
                                << "CASE "
                                << "  WHEN ior.period = 0 THEN 0 "
                                << "  ELSE 100 * SUM(ipm.price) * 12 / ior.period END, "
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
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        clear();
      }
      catch (std::exception& ex) {
        LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
        clear();
      }

    }    
    virtual void reload() {

        // auto_ptr just to avoid changing -> into . in 1000 places :)
        std::auto_ptr<autoDB> db(new autoDB());
        if (!db->OpenDatabase(Database::Manager::getConnectionString())) {
            LOGGER(PACKAGE).error(" autoDB: Failed to open the database. ");
            throw SQL_ERROR();
        }

        clearList();
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
      SQL_ID_FILTER(where,"i.id",idFilter);
      SQL_ID_FILTER(where,"i.registrarid",registrarFilter);
      SQL_ID_FILTER(where,"i.zone",zoneFilter);
      if (typeFilter && typeFilter<=2) {
        from << ", invoice_prefix ip ";
        where << "AND i.prefix_type=ip.id ";
        SQL_ID_FILTER_FILL(where,"ip.typ",typeFilter-1);
      }
      if (!varSymbolFilter.empty() || !registrarHandleFilter.empty()) {
        from << ", registrar r ";
        where << "AND i.registrarid=r.id ";
        if (!varSymbolFilter.empty())
        SQL_WILDCARD_FILTER_FILL(where,"TRIM(r.varsymb)",varSymbolFilter);
        if (!registrarHandleFilter.empty())
        SQL_WILDCARD_FILTER_FILL(where,"r.handle",registrarHandleFilter);
      }
      SQL_WILDCARD_FILTER(where,"i.prefix",numberFilter);
      SQL_DATE_FILTER(where,"i.crdate",crDateFilter);
      SQL_TIME_FILTER(where,"i.taxdate",taxDateFilter);
      switch (archiveFilter) {
        case AF_IGNORE: break;
        case AF_SET: where << "AND NOT(i.file ISNULL) "; break;
        case AF_UNSET: where << "AND i.file ISNULL "; break;
      }
      if (objectIdFilter) {
        from << ", invoice_object_registry ior ";
        where << "AND i.id=ior.invoiceid ";
        SQL_ID_FILTER_FILL(where,"ior.objectid",objectIdFilter);
      }
      if (!objectNameFilter.empty()) {
        from << ", invoice_object_registry iorh, object_registry obr ";
        where << "AND i.id=iorh.invoiceid AND obr.id=iorh.objectid ";
        SQL_WILDCARD_FILTER_FILL(where,"obr.name",objectNameFilter);
      }

      if (!advanceNumberFilter.empty()) {
        from << ", invoice_object_registry ior2 "
        << ", invoice_object_registry_price_map iorpm "
        << ", invoice advi ";
        where << "AND i.id=ior2.invoiceid "
        << "AND iorpm.id=ior2.id AND iorpm.invoiceid=advi.id ";
        SQL_WILDCARD_FILTER_FILL(where,"advi.prefix",advanceNumberFilter);
      }
      // complete sql end do the query
      sql << from.rdbuf() << where.rdbuf();
      if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
      if (!db->ExecSQL("ANALYZE tmp_invoice_filter_result"))
      throw SQL_ERROR();
      // initialize list of invoices using temporary table 
      if (!db->ExecSelect(
              "SELECT "
              " i.id, i.zone, i.crdate::timestamptz AT TIME ZONE 'Europe/Prague',"
              " i.taxdate, ig.fromdate, "
              " ig.todate, ip.typ, i.prefix, i.registrarid, i.credit*100, "
              " i.price*100, i.vat, i.total*100, i.totalvat*100, "
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
              " ORDER BY crdate DESC "
          )) throw SQL_ERROR();
      for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++)
      data_.push_back(new InvoiceImpl(db.get(),man,i));
      db->FreeSelect();
      // append list of actions to all selected invoices
      // it handle situation when action come from source advance invoices
      // with different vat rates by grouping
      // this is ignored on partial load
      if (!partialLoad) {
        if (!db->ExecSelect(
                "SELECT "
                " it.id, o.name, ior.crdate::timestamptz AT TIME ZONE 'Europe/Prague',"
                " ior.exdate, ior.operation, ior.period, "
                " CASE "
                "  WHEN ior.period=0 THEN 0 "
                "  ELSE 100*SUM(ipm.price)*12/ior.period END, "
                " SUM(ipm.price)*100, o.id, i.vat "
                "FROM "
                " tmp_invoice_filter_result it "
                " JOIN invoice_object_registry ior ON (it.id=ior.invoiceid) "
                " JOIN object_registry o ON (ior.objectid=o.id) "
                " JOIN invoice_object_registry_price_map ipm ON (ior.id=ipm.id) "
                " JOIN invoice i ON (ipm.invoiceid=i.id) "
                "GROUP BY "
                " it.id, o.name, ior.crdate, ior.exdate, ior.operation, "
                " ior.period, o.id, i.vat "
            )) throw SQL_ERROR();

        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          InvoiceImpl *inv = dynamic_cast<InvoiceImpl*>(findId(STR_TO_ID(db->GetFieldValue(i,0))));

          // addAction only uses db, it doesnt want to own it
          if (inv) inv->addAction(db.get(),i);
          else {
            LOGGER(PACKAGE).error(" dynamic_cast failed for Invoice. ");
            // TODO: log error - more specific error
          }
        }
        db->FreeSelect();
      }
      // append list of sources to all selected invoices
      if (!db->ExecSelect(
              "SELECT "
              " it.id, sri.prefix, ipm.credit*100, ipm.balance*100, sri.id, "
              " sri.total*100, sri.vat, sri.totalvat*100, sri.crdate "
              "FROM "
              " tmp_invoice_filter_result it "
              " JOIN invoice_credit_payment_map ipm ON (it.id=ipm.invoiceid) "
              " JOIN invoice sri ON (ipm.ainvoiceid=sri.id) "
          )) throw SQL_ERROR();
      for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
        InvoiceImpl *inv = dynamic_cast<InvoiceImpl*>(findId(STR_TO_ID(db->GetFieldValue(i,0))));
        if (inv) inv->addSource(db.get(),i);
        else {
          // TODO: log error - more specific
            LOGGER(PACKAGE).error(" autoDB: Failed to open the database. ");
            throw SQL_ERROR();
        }
      }
      db->FreeSelect();
      // delete temporary table
      if (!db->ExecSQL("DROP TABLE tmp_invoice_filter_result "))
      throw SQL_ERROR();
    }
    
    
    /// export all invoices on the list using given exporter
    void doExport(Exporter *_exporter) {
      Iterator it = data_.begin();
      for (; it != data_.end(); ++it) {
        InvoiceImpl *invoice = dynamic_cast<InvoiceImpl*>(*it);
        if (invoice)
          invoice->doExport(_exporter);
      }
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
    Database::Connection conn;    
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
      conn.exec(sql.str());
    }
    
  public:
    Mails(Mailer::Manager *_mm) : mm(_mm), conn(Database::Manager::acquire()) {
    }
    /// send all mails and store information about sending
    void send() throw (SQL_ERROR) {
      for (unsigned i=0; i<items.size(); i++) {
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
            LOGGER(PACKAGE).error(" Error while send mail in Mails class. ");
            throw SQL_ERROR();
        }
        store(i);
      }
    }
 
    void load() throw (SQL_ERROR) {

        std::auto_ptr<autoDB> db(new autoDB());
        if(!db->OpenDatabase(Database::Manager::getConnectionString())) {
            LOGGER(PACKAGE).error(" autoDB: Failed to open the database. ");
            throw SQL_ERROR();
        }

      std::stringstream sql;

      sql << "SELECT r.email, g.fromdate, g.todate, "
      << "i.file, i.fileXML, g.id, i.id, z.fqdn "
      << "FROM registrar r, invoice i "
      << "LEFT JOIN invoice_generation g ON (g.invoiceid=i.id) "
      << "LEFT JOIN invoice_mails im ON (im.invoiceid=i.id) "
      << "LEFT JOIN zone z ON (z.id = i.zone) "
      << "WHERE i.registrarid=r.id "
      << "AND im.mailid ISNULL "
      << "UNION "
      << "SELECT r.email, g.fromdate, g.todate, NULL, NULL, g.id, "
      << "NULL, z.fqdn "
      << "FROM registrar r, invoice_generation g "
      << "LEFT JOIN invoice_mails im ON (im.genid=g.id) "
      << "LEFT JOIN zone z ON (z.id = g.zone) "
      << "WHERE g.registrarid=r.id AND g.invoiceid ISNULL "
      << "AND im.mailid ISNULL "
      << "AND NOT(r.email ISNULL OR TRIM(r.email)='')";

      if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
      for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++)
      items.push_back(Item(
              db->GetFieldValue(i,0),
              MAKE_DATE(i,1),
              MAKE_DATE(i,2),
              STR_TO_ID(db->GetFieldValue(i,3)),
              STR_TO_ID(db->GetFieldValue(i,4)),
              STR_TO_ID(db->GetFieldValue(i,5)),
              STR_TO_ID(db->GetFieldValue(i,6)),
              (TID)0,
              db->GetFieldValue(i, 7)
          ));
      db->FreeSelect();
    }

  }; // Mails
  

  void ManagerImpl::initVATList()  {

    std::auto_ptr<autoDB> db(new autoDB());
    if(!db->OpenDatabase(Database::Manager::getConnectionString())) {
        LOGGER(PACKAGE).error(" autoDB: Failed to open the database. ");
       throw SQL_ERROR();
    }

    if (vatList.empty()) {
      if (!db->ExecSelect("SELECT vat, 10000*koef, valid_to FROM price_vat"))
      throw SQL_ERROR();
      for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
        vatList.push_back(
            VAT(
                atoi(db->GetFieldValue(i,0)),
                atoi(db->GetFieldValue(i,1)),
                MAKE_DATE(i,2)
            )
        );
      }
      db->FreeSelect();
    }
  }


  Money ManagerImpl::countVAT(Money price, unsigned vatRate, bool base) {
    const VAT *v = getVAT(vatRate);
    unsigned coef = v ? v->koef : 0;
    return price * coef / (10000 - (base ? coef : 0));
  }
  
  const VAT * ManagerImpl::getVAT(unsigned rate) const {
    // late initialization would brake constness
    ((ManagerImpl *)this)->initVATList();
    std::vector<VAT>::const_iterator ci = find(
        vatList.begin(),vatList.end(),rate
    );
    return ci == vatList.end() ? NULL : &(*ci);
  }
  
  void ManagerImpl::archiveInvoices(bool send) const {
      
      if(docman == NULL || mailman == NULL) {
        LOGGER(PACKAGE).error("archiveInvoices: No docman or mailman specified in c-tor. ");    
      }
      
    try {
      // archive unarchived invoices
      ExporterArchiver exporter(docman);
      ListImpl l((ManagerImpl *)this);






      l.setArchivedFilter(ListImpl::AF_UNSET);
      l.reload();
      l.doExport(&exporter);
      if (send) {
        Mails m(mailman);
        m.load();
        m.send();
      }
    }
    catch (...) {
      LOGGER(PACKAGE).error("Exception in archiveInvoices.");
      //TODO: LOG more specific ERROR
    }
  }
  
  List* ManagerImpl::createList() const {
    return new ListImpl((ManagerImpl *)this);
    // return new ListImpl(conn_, (ManagerImpl *)this);
  }
  
  Money ManagerImpl::getCreditByZone(const std::string& registrarHandle, TID zone) {
      Database::Connection conn = Database::Manager::acquire();
    std::stringstream sql;
    sql << "SELECT SUM(credit)*100 "
    << "FROM invoice i JOIN registrar r ON (i.registrarid=r.id) "
    << "WHERE i.zone=" << zone << " AND r.handle='"
    << registrarHandle << "'";

    Database::Result res = conn.exec(sql.str());
    Money result = (long)res[0][0];
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
}; // Register
