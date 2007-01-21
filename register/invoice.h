#ifndef INVOICE_H_
#define INVOICE_H_

#include "types.h"
#include "exceptions.h"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

using namespace boost::posix_time;
using namespace boost::gregorian;

class DB;
namespace Register
{
  namespace Invoicing
  {
    /// money type
    typedef unsigned long long Money;
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
      virtual const std::string& getName() const = 0;
      virtual const std::string& getFullname() const = 0;
      virtual const std::string& getStreet() const = 0;
      virtual const std::string& getCity() const = 0;
      virtual const std::string& getZip() const = 0;
      virtual const std::string& getICO() const = 0;
      virtual const std::string& getVatNumber() const = 0;
      virtual const std::string& getRegistration() const = 0;
      virtual const std::string& getReclamation() const = 0;
      virtual const std::string& getEmail() const = 0;
      virtual const std::string& getURL() const = 0;
    };
    class Invoice
    {
     public:
      virtual ~Invoice() {}
      virtual const Subject* getSupplier() const = 0;
      virtual const Subject* getClient() const = 0;
      virtual TID getId() const = 0;
      virtual TID getZone() const = 0;
      virtual ptime getCrTime() const = 0;
      virtual date getTaxDate() const = 0;
      virtual time_period getAccountPeriod() const = 0;
      virtual Type getType() const = 0;
      virtual unsigned getNumber() const = 0;
      virtual TID getRegistrar() const = 0;
      virtual Money getCredit() const = 0;
      virtual Money getPrice() const = 0;
      virtual short getVatRate() const = 0;
      virtual Money getTotal() const = 0;
      virtual Money getTotalVAT() const = 0;
    };
    class InvoiceList
    {
      
    };
    class Manager
    {
     public:
      virtual ~Manager() {}
      /// find unarchived invoices and archive then in PDF format
      virtual void archiveInvoices() const = 0;
      /// factory method
      static Manager *create(DB *db);
    }; // Manager
  }; // Invoicing
}; // Register

#endif // INVOICE_H_
