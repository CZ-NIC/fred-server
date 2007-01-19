#include "invoice.h"
#include "dbsql.h" 
#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>

#include <boost/date_time/gregorian/gregorian.hpp>

using namespace boost::gregorian;

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
    class InvoiceImpl;
    /// exporter interface 
    class Exporter
    {
     public:
      virtual ~Exporter() {}
      virtual void doExport(InvoiceImpl *) = 0;
    };
    // exporter that export xml of invoice into given stream
    class ExporterXML : public Exporter
    {
      std::ostream& out;
     public:
      ExporterXML(std::ostream& _out) : out(_out)
      {}
      virtual void doExport(InvoiceImpl *)
      {
        out << "<invoice>";
        
        out << "</invoice>";
      }      
    };
    class InvoiceImpl : public Invoice
    {
      TID id;
      TID zone;
      ptime crTime;
      date taxDate;
      time_period accountPeriod;
      Type type;
      unsigned number;
      TID registrar;
      Money credit;
      Money price;
      short vatRate;
      Money total;
      Money totalVAT;
     public:
      TID getId() const
      {
        return id;
      }
      TID getZone() const
      {
        return zone;
      }
      ptime getCrTime() const
      {
        return crTime;
      }
      date getTaxDate() const
      {
        return taxDate;
      }
      time_period getAccountPeriod() const
      {
        return accountPeriod;
      }
      Type getType() const
      {
        return type;
      }
      unsigned getNumber() const
      {
        return number;
      }
      TID getRegistrar() const
      {
        return registrar;
      }
      Money getCredit() const
      {
        return credit;
      }
      Money getPrice() const
      {
        return price;
      }
      short getVatRate() const
      {
        return vatRate;
      }
      Money getTotal() const
      {
        return total;
      }
      Money getTotalVAT() const
      {
        return totalVAT;
      }
      InvoiceImpl(DBsql *_db, unsigned line) :
        accountPeriod(ptime(neg_infin),ptime(pos_infin))
      {        
      }
      /// export invoice using given exporter
      void doExport(Exporter *exp) const
      {
        exp->doExport(const_cast<InvoiceImpl*>(this));
      }
    }; 
    class InvoiceListImpl : public InvoiceList
    {
      typedef std::vector<InvoiceImpl *> InvoicesType; ///< invoice list type
      InvoicesType invoices; ///< list of invoices for given filter
      DB *db;
     public:
      InvoiceListImpl(DB *_db) : db(_db)
      {}
      /// export all invoices on the list using given exporter
      void doExport(Exporter *exp) const
      {
        for_each(
          invoices.begin(),invoices.end(),
          std::bind2nd(std::mem_fun(&InvoiceImpl::doExport),exp)
        );
      }
    }; // InvoiceListImpl 
    /// implementation for Manager interface
    class ManagerImpl : public Manager
    {
      DBsql *db;
     public:
      ManagerImpl(DBsql *_db) : db(_db)
      {}
      /// find unarchived invoices and archive then in PDF format
      void archiveInvoices() const
      {
      }
    }; // ManagerImpl
    Manager *Manager::create(DBsql *db)
    {
      return new ManagerImpl(db);
    }    
  }; // Invoicing
}; // Register
