#ifndef INVOICE_H_
#define INVOICE_H_

#include "types.h"
#include "exceptions.h"
#include "documents.h"
#include "mailer.h"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

class DB;
namespace Register
{
  namespace Invoicing
  {
    /// money type
    typedef long long Money;
    /// invoice type
    enum Type {
      IT_DEPOSIT, ///< depositing invoice
      IT_ACCOUNT  ///< accounting invoice
    };
    /// subject of invoicing (supplier or client)
    class Subject
    {
     protected:
      virtual ~Subject() {}
     public:
      virtual const std::string& getHandle() const = 0;
      virtual const std::string& getName() const = 0;
      virtual const std::string& getFullname() const = 0;
      virtual const std::string& getStreet() const = 0;
      virtual const std::string& getCity() const = 0;
      virtual const std::string& getZip() const = 0;
      virtual const std::string& getICO() const = 0;
      virtual const std::string& getVatNumber() const = 0;
      virtual bool getVatApply() const = 0;
      virtual const std::string& getRegistration() const = 0;
      virtual const std::string& getReclamation() const = 0;
      virtual const std::string& getEmail() const = 0;
      virtual const std::string& getURL() const = 0;
    };
    class PaymentSource   
    {
     protected:
      virtual ~PaymentSource() {}      
     public:
      virtual unsigned long getNumber() const = 0;
      virtual Money getPrice() const = 0;
      virtual Money getCredit() const = 0;
      virtual TID getId() const = 0;
    };
    enum PaymentActionType {
      PAT_CREATE_DOMAIN,
      PAT_RENEW_DOMAIN
    };
    class PaymentAction
    {
     protected:
      virtual ~PaymentAction() {}
     public:
      virtual TID getObjectId() const = 0;
      virtual const std::string& getObjectName() const = 0;
      virtual boost::posix_time::ptime getActionTime() const = 0;
      virtual boost::gregorian::date getExDate() const = 0;
      virtual PaymentActionType getAction() const = 0;
      virtual unsigned getUnitsCount() const = 0;
      virtual Money getPricePerUnit() const = 0;
      virtual Money getPrice() const = 0;
    };
    class Invoice
    {
     public:
      virtual ~Invoice() {}
      virtual const Subject* getSupplier() const = 0;
      virtual const Subject* getClient() const = 0;
      virtual TID getId() const = 0;
      virtual TID getZone() const = 0;
      virtual boost::posix_time::ptime getCrTime() const = 0;
      virtual boost::gregorian::date getTaxDate() const = 0;
      virtual boost::posix_time::time_period getAccountPeriod() const = 0;
      virtual Type getType() const = 0;
      virtual unsigned long getNumber() const = 0;
      virtual TID getRegistrar() const = 0;
      virtual Money getCredit() const = 0;
      virtual Money getPrice() const = 0;
      virtual short getVatRate() const = 0;
      virtual Money getTotal() const = 0;
      virtual Money getTotalVAT() const = 0;
      virtual const std::string& getVarSymbol() const = 0;
      virtual TID getFilePDF() const = 0;
      virtual TID getFileXML() const = 0;
      virtual unsigned getSourceCount() const = 0;
      virtual const PaymentSource *getSource(unsigned idx) const = 0;
      virtual unsigned getActionCount() const = 0;
      virtual const PaymentAction *getAction(unsigned idx) const = 0;
    };
    class InvoiceList
    {
     public:
      /// publice destructor
      virtual ~InvoiceList() {};
      /// type for setting archive filter
      enum ArchiveFilter
      {
        AF_IGNORE, ///< ignore this filter (NULL-value)
        AF_SET, ///< invoice is archived
        AF_UNSET ///< invoice is still unarchived
      };
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
      virtual void setCrDateFilter(
        const boost::posix_time::time_period& crDatePeriod
      ) = 0;
      /// filter for taxDate
      virtual void setTaxDateFilter(
        const boost::posix_time::time_period& taxDatePeriod
      ) = 0;
      /// filter for existance of archived file
      virtual void setArchivedFilter(ArchiveFilter archive) = 0;
      /// filter for object attached to invoice by id
      virtual void setObjectIdFilter(TID objectId) = 0;
      /// filter for object attached to invoice by name
      virtual void setObjectNameFilter(const std::string& objectName) = 0;
      /// filter for account invoices with selected advance invoice source
      virtual void setAdvanceNumberFilter(const std::string& number) = 0;
      /// reload invoices with selected filter
      virtual void reload() throw (SQL_ERROR) = 0;
      /// return count of invoices in list 
      virtual unsigned long getCount() const = 0;
      /// return invoice by index
      virtual Invoice *get(unsigned idx) const = 0;
      /// clear filter settings
      virtual void clearFilter() = 0;      
      /// xml output of all invoices in list
      virtual void exportXML(std::ostream& out) = 0;     
    };
    /// facade of invoicing subsystem
    class Manager
    {
     public:
      /// public destructor - client is responsible for destroying manager
      virtual ~Manager() {}
      /// find unarchived invoices and archive then in PDF format
      virtual void archiveInvoices() const = 0;
      /// create empty list of invoices
      virtual InvoiceList* createList() const = 0;
      /// factory method
      static Manager *create(
        DB *db, Document::Manager *docman, Mailer::Manager *mailman
      );
    }; // Manager
  }; // Invoicing
}; // Register

#endif // INVOICE_H_
