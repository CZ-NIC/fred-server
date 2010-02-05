#ifndef INVOICE_H_
#define INVOICE_H_

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

/*
#include "common_object.h"
*/
#include "common_object.h"
#include "object.h"
#include "types.h"
#include "exceptions.h"
#include "documents.h"
#include "mailer.h"
#include "db_settings.h"
#include "model/model_filters.h"
#include "old_utils/dbsql.h" 

class DB;

namespace Register {
namespace Invoicing {

/// Member identification (i.e. for sorting) 
enum MemberType {
  MT_CRTIME,
  MT_CRDATE,
  MT_NUMBER,
  MT_REGISTRAR,
  MT_TOTAL,
  MT_CREDIT,
  MT_TYPE,
  MT_ZONE,
  MT_PRICE
};

/// money type
typedef long long Money;
/// invoice type
// duplicity from ccReg.h
enum Type {
  IT_DEPOSIT, ///< depositing invoice
  IT_ACCOUNT, ///< accounting invoice
  IT_NONE
};
std::string Type2Str(Type _type);

/// subject of invoicing (supplier or client)
class Subject {
protected:
  virtual ~Subject() {
  }
public:
  virtual TID getId() const = 0;
  virtual const std::string& getHandle() const = 0;
  virtual const std::string& getName() const = 0;
  virtual const std::string& getFullname() const = 0;
  virtual const std::string& getStreet() const = 0;
  virtual const std::string& getCity() const = 0;
  virtual const std::string& getZip() const = 0;
  virtual const std::string& getCountry() const = 0;
  virtual const std::string& getICO() const = 0;
  virtual const std::string& getVatNumber() const = 0;
  virtual bool getVatApply() const = 0;
  virtual const std::string& getRegistration() const = 0;
  virtual const std::string& getReclamation() const = 0;
  virtual const std::string& getEmail() const = 0;
  virtual const std::string& getURL() const = 0;
  virtual const std::string& getPhone() const = 0;
  virtual const std::string& getFax() const = 0;
};
/// description of any payment transfer with vat
class Payment {
protected:
  virtual ~Payment() {
  }
public:
  // base tax money amount  
  virtual Money getPrice() const = 0;
  // vat rate in percents
  virtual unsigned getVatRate() const = 0;
  // counted vat (approximatly price * vatRate / 100)
  virtual Money getVat() const = 0;
  // sum of price and vat
  virtual Money getPriceWithVat() const = 0;
};
class PaymentSource : virtual public Payment {
protected:
  virtual ~PaymentSource() {
  }
public:
  virtual TID getId() const = 0;
  /// number of source advance invoice 
  virtual unsigned long long getNumber() const = 0;
  /// remaining credit on source invoice
  virtual Money getCredit() const = 0;
  /// total price withput vat of source invoice
  virtual Money getTotalPrice() const = 0;
  /// total vat of source invoice
  virtual Money getTotalVat() const = 0;
  /// total price with vat of source invoice
  virtual Money getTotalPriceWithVat() const = 0;
  /// creation time of source invoice
  virtual boost::posix_time::ptime getCrTime() const = 0;
};

enum PaymentActionType {
  PAT_CREATE_DOMAIN,
  PAT_RENEW_DOMAIN
};
std::string PaymentActionType2Str(PaymentActionType type);

class PaymentAction : virtual public Payment {
protected:
  virtual ~PaymentAction() {
  }
public:
  /// id of object on which action was performed
  virtual TID getObjectId() const = 0;
  /// name of object on which action was performed
  virtual const std::string& getObjectName() const = 0;
  /// time of action 
  virtual boost::posix_time::ptime getActionTime() const = 0;
  /// exdate when operation was renew otherwise (not-a-date)
  virtual boost::gregorian::date getExDate() const = 0;
  /// type of action (creation or registration extension)
  virtual PaymentActionType getAction() const = 0;
  /// number of month for extenstion otherwise 0
  virtual unsigned getUnitsCount() const = 0;
  /// price (without vat) per unit
  virtual Money getPricePerUnit() const = 0;

  //// new interface - implement
  virtual std::string getActionStr() const = 0;
};
/// iterator over list of total price partitioned by years
class AnnualPartitioning : virtual public Payment {
protected:
  virtual ~AnnualPartitioning() {
  }
public:
  /// reset iterator to first record
  virtual void resetIterator(unsigned vatRate) = 0;
  /// test whether there are any records left 
  virtual bool end() const = 0;
  /// pass to next record
  virtual void next() = 0;
  /// access method for year at current record 
  virtual unsigned getYear() const = 0;
};

class Invoice : virtual public Register::CommonObject {
public:
  virtual ~Invoice() {
  }
  virtual const Subject* getSupplier() const = 0;
  virtual const Subject* getClient() const = 0;
  virtual TID getZone() const = 0;
  virtual const std::string& getZoneName() const = 0;
  virtual boost::posix_time::ptime getCrTime() const = 0;
  virtual boost::gregorian::date getTaxDate() const = 0;
  virtual boost::gregorian::date_period getAccountPeriod() const = 0;
  virtual Type getType() const = 0;
  virtual unsigned long long getNumber() const = 0;
  virtual TID getRegistrar() const = 0;
  virtual Money getCredit() const = 0;
  virtual Money getPrice() const = 0;
  virtual short getVatRate() const = 0;
  virtual Money getTotal() const = 0;
  virtual Money getTotalVAT() const = 0;
  virtual const std::string& getVarSymbol() const = 0;
  virtual TID getFilePDF() const = 0;
  virtual TID getFileXML() const = 0;
  virtual std::string getFileNamePDF() const = 0;
  virtual std::string getFileNameXML() const = 0;
  virtual unsigned getSourceCount() const = 0;
  virtual const PaymentSource *getSource(unsigned idx) const = 0;
  virtual unsigned getActionCount() const = 0;
  virtual const PaymentAction *getAction(unsigned idx) const = 0;
  virtual AnnualPartitioning *getAnnualPartitioning() = 0;
  virtual unsigned getPaymentCount() const = 0;
  virtual const Payment *getPaymentByIdx(unsigned idx) const = 0;

    // //////////////////// new interface (partially from model_invoices)
    
  
    virtual TID getZoneId() const = 0;
    virtual boost::posix_time::ptime getCrDate() const = 0;
    // const Database::DateTime getCrDate() const = 0;
    // const Database::Date getTaxDate() const = 0;
    virtual TID getPrefix() const = 0;
    virtual TID getRegistrarId() const = 0;
    virtual const int getVat() const = 0;
    virtual const Database::Money getTotalVat() const = 0;
    // const TID getPrefixTypeId() const = 0;
    virtual TID getFileId() const = 0;
    virtual std::string getFileHandle() const = 0;
    virtual TID getFileXmlId() const = 0;
    virtual std::string getFileXmlHandle() const = 0;
    virtual std::string getZoneFqdn() const = 0;

    virtual const Payment *getPayment(const unsigned int &index) const = 0;
};

class List : virtual public Register::CommonList {
public:
  /// publice destructor
  virtual ~List() {
  }
  ;
  /// type for setting archive filter
  enum ArchiveFilter
  {
    AF_IGNORE, ///< ignore this filter (NULL-value)
    AF_SET, ///< invoice is archived
    AF_UNSET ///< invoice is still unarchived
  };
  
  virtual const char* getTempTableName() const = 0;
  /// reload invoices with selected filter
  virtual void reload() = 0;
  /// new reload method
  virtual void reload(Database::Filters::Union& _uf) = 0;
  /// subsequential reload will not load any action 
  virtual void setPartialLoad(bool partialLoad) = 0;
  /// return invoice by index
  virtual Invoice *get(unsigned _idx) const = 0;
  /// clear filter settings
  virtual void clearFilter() = 0;
  /// xml output of all invoices in list
  virtual void exportXML(std::ostream& out) = 0;
  /// sort by column
  virtual void sort(MemberType _member, bool _asc) = 0;

  /// filter for invoice id
  virtual void setIdFilter(TID id) = 0;
  /// filter for registrar recieving invoice
  virtual void setRegistrarFilter(TID registrar) = 0;
  /// filter for registrar recieving invoice by handle
  virtual void setRegistrarHandleFilter(const std::string& handle) = 0;
  /// filter for id of associated zone 
  virtual void setZoneFilter(TID zone) = 0;
  /// filter for invoice type (advance=1 or normal=2)
  virtual void setTypeFilter(unsigned type) = 0;
  /// filter for variable symbol of payment
  virtual void setVarSymbolFilter(const std::string& varSymbol) = 0;
  /// filter for invoice number
  virtual void setNumberFilter(const std::string& number) = 0;
  /// filter for crDate
  virtual void
      setCrDateFilter(const boost::posix_time::time_period& crDatePeriod) = 0;
  /// filter for taxDate
  virtual void
      setTaxDateFilter(const boost::posix_time::time_period& taxDatePeriod) = 0;
  /// filter for existance of archived file
  virtual void setArchivedFilter(ArchiveFilter archive) = 0;
  /// filter for object attached to invoice by id
  virtual void setObjectIdFilter(TID objectId) = 0;
  /// filter for object attached to invoice by name
  virtual void setObjectNameFilter(const std::string& objectName) = 0;
  /// filter for account invoices with selected advance invoice source
  virtual void setAdvanceNumberFilter(const std::string& number) = 0;
  
  /// from CommonList; propably will be removed in future
  virtual void makeQuery(bool, bool, std::stringstream&) const = 0;  

  virtual unsigned int getSize() const = 0;

  };

/// facade of invoicing subsystem
class Manager {
public:
  /// public destructor - client is responsible for destroying manager
  virtual ~Manager() {
  }
  /// find unarchived invoices and archive then in PDF format
  virtual void archiveInvoices(bool send) const = 0;
  /// create empty list of invoices
  virtual List* createList() const = 0;
  /// return credit for registrar by zone
  virtual Money getCreditByZone(const std::string& registrarHandle, 
                                TID zone) = 0;
  /// factory method
  /*
  static Manager *create(DB *db,
                         Document::Manager *docman,
                         Mailer::Manager *mailman);
   */
  
  static Manager *create(
                         Document::Manager *_doc_manager,
                         Mailer::Manager *_mail_manager);

  static Manager *create();

  virtual bool insertInvoicePrefix(unsigned long long zoneId,
          int type, int year, unsigned long long prefix) = 0;
  virtual bool insertInvoicePrefix(const std::string &zoneName,
          int type, int year, unsigned long long prefix) = 0;

  // added methods
  virtual  int createDepositInvoice(Database::Date date, int zoneId, int registrarId, long price) = 0; 
  // nnnn
  virtual bool domainBilling(
            DB *db, 
            const Database::ID &zone,
            const Database::ID &registrar,
            const Database::ID &objectId,
            const Database::Date &exDate,
            const int &units_count,
            bool renew) = 0;

  virtual bool factoring_all(const char *database, const char *zone_fqdn, const char *taxdateStr, const char *todateStr) = 0;
  virtual int factoring(const char *database, const char *registrarHandle, const char *zone_fqdn, const char *taxdateStr, const char *todateStr) = 0;

}; // Manager
}
; // Invoicing
}
; // Register

#endif // INVOICE_H_
