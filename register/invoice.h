#ifndef INVOICE_H_
#define INVOICE_H_

#include "types.h"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>

using namespace boost::posix_time;

class DBsql;
namespace Register
{
  namespace Invoicing
  {
    class Invoice
    {
      
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
      static Manager *create(DBsql *db);
    }; // Manager
  }; // Invoicing
}; // Register

#endif // INVOICE_H_
