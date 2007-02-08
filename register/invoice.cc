#include "dbsql.h" 
#include "invoice.h"
#include "documents.h"
#include "sql.h"
#include <memory>
#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <boost/date_time/posix_time/time_parsers.hpp>

#define MAKE_TIME_DEF(ROW,COL,DEF)  \
  (boost::posix_time::ptime(db->IsNotNull(ROW,COL) ? \
  boost::posix_time::time_from_string(\
  db->GetFieldValue(ROW,COL)) : DEF))         
#define MAKE_TIME(ROW,COL) \
  MAKE_TIME_DEF(ROW,COL,boost::posix_time::not_a_date_time)         
#define MAKE_TIME_NEG(ROW,COL) \
  MAKE_TIME_DEF(ROW,COL,boost::posix_time::neg_infin)         
#define MAKE_TIME_POS(ROW,COL) \
  MAKE_TIME_DEF(ROW,COL,boost::posix_time::pos_infin)         
#define MAKE_DATE(ROW,COL)  \
 (boost::gregorian::date(db->IsNotNull(ROW,COL) ? \
 boost::gregorian::from_string(\
 db->GetFieldValue(ROW,COL)) : \
 (boost::gregorian::date)boost::gregorian::not_a_date_time))

#define STR_TO_MONEY(x) atol(x)

using namespace boost::posix_time;

namespace Register
{
  namespace Invoicing
  {
    class SubjectImpl : public Subject {
      std::string name;
      std::string fullname;
      std::string street;
      std::string city;
      std::string zip;
      std::string ico;
      std::string vatNumber;
      std::string registration;
      std::string reclamation;
      std::string email;
      std::string url;
     public:
      SubjectImpl(
        const std::string& _name, const std::string& _fullname,
        const std::string& _street, const std::string& _city,
        const std::string& _zip, const std::string& _ico,
        const std::string& _vatNumber, const std::string& _registration,
        const std::string& _reclamation, const std::string& _email,
        const std::string& _url
      ) : 
        name(_name), fullname(_fullname), street(_street), city(_city),
        zip(_zip), ico(_ico), vatNumber(_vatNumber),
        registration(_registration), reclamation(_reclamation),
        email(_email), url(_url)
      {}      
      const std::string& getName() const { return name; }
      const std::string& getFullname() const { return fullname; }
      const std::string& getStreet() const { return street; }
      const std::string& getCity() const { return city; }
      const std::string& getZip() const { return zip; }
      const std::string& getICO() const { return ico; }
      const std::string& getVatNumber() const { return vatNumber; }
      const std::string& getRegistration() const { return registration; }
      const std::string& getReclamation() const { return reclamation; }
      const std::string& getEmail() const { return email; }
      const std::string& getURL() const { return url; }
    };
    class InvoiceImpl;
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //   Exporter
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    /// common exporter interface 
    class Exporter
    {
     public:
      virtual ~Exporter() {}
      virtual void doExport(Invoice *) = 0;
    };
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //   ExporterXML
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    #define TAGSTART(tag) "<"#tag">"
    #define TAGEND(tag) "</"#tag">"
    #define TAG(tag,f) TAGSTART(tag) << f << TAGEND(tag)
    #define OUTMONEY(f) (f)/100 << "." << \
                        std::setfill('0') << std::setw(2) << (f)%100
    // builder that export xml of invoice into given stream
    class ExporterXML : public Exporter
    {
      std::ostream& out;
      bool xmlDec; ///< whether to include xml declaration 
     public:
      ExporterXML(std::ostream& _out, bool _xmlDec) : 
        out(_out), xmlDec(_xmlDec)
      {}
      std::ostream& doExport(const Subject* s)
      {
        out << TAG(name,s->getName())
            << TAG(fullname,s->getFullname())
            << TAGSTART(address)
            << TAG(street,s->getStreet())
            << TAG(city,s->getCity())
            << TAG(zip,s->getZip())
            << TAGEND(address)
            << TAG(ico,s->getICO())
            << TAG(vat_number,s->getVatNumber())
            << TAG(registration,s->getRegistration())
            << TAG(reclamation,s->getReclamation())
            << TAG(url,s->getURL())
            << TAG(email,s->getEmail());
        return out;
      }
      virtual void doExport(Invoice *i)
      {
        // setting locale for proper date format
        out.imbue(std::locale(
          // must be default locale (do not set locale("") because of
          // unpredictable formatting behavior)
          out.getloc(),
          new boost::gregorian::date_facet("%Y-%m-%d")
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
        if (i->getType() != IT_DEPOSIT)
          out << TAG(tax_point,i->getTaxDate());  
        else
          out << TAG(advance_payment_date,i->getTaxDate());
        out << TAG(vs,i->getVarSymbol())
            << TAGEND(payment)
            << TAGSTART(delivery)
            << TAGSTART(entry)
            << TAG(vatperc,i->getVatRate())
            << TAG(basetax,OUTMONEY(i->getTotal()))
            << TAG(vat,OUTMONEY(i->getTotalVAT()))
            << TAG(total,OUTMONEY(i->getPrice()))
            << TAGEND(entry)
            << TAGSTART(sumarize)
            << TAG(total,OUTMONEY(i->getPrice()))
            << TAG(paid,OUTMONEY(i->getPrice()))
            << TAG(to_be_paid,OUTMONEY(0))
            << TAGEND(sumarize)
            << TAGEND(delivery)
/*            
            << TAGSTART(appendix)
            << TAGSTART(item)
            << TAG(subject,"Zálohová platba")
            << TAG(code,"")
            << TAG(date,i->getCrTime().date())
            << TAG(count,1)
            << TAG(vatperc,i->getTotalVAT())
            << TAG(price,i->getPrice())
            << TAG(total,i->getTotal())
            << TAGEND(item)
            << TAGEND(appendix)
            << TAGSTART(sumarize_items)
            << TAG(total,i->getPrice())
            << TAGEND(sumarize_items)
 */
            << TAGEND(invoice);
      }      
    };
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //   ExporterPDF
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // builder that create pdf of invoice and store it into filesystem
    class ExporterPDF : public Exporter
    {
      Document::Manager *docman;
     public:
      ExporterPDF(Document::Manager *_docman) : docman(_docman) {}
      virtual void doExport(Invoice *i)
      {
        std::stringstream filenamePDF;
        filenamePDF << "/tmp/f" << i->getNumber() << ".pdf";
        std::fstream outf(filenamePDF.str().c_str(),std::ios::out);
        std::auto_ptr<Document::Generator> g(
          docman->createOutputGenerator(
            i->getType() == IT_DEPOSIT ? 
              Document::GT_ADVANCE_INVOICE_PDF :
              Document::GT_INVOICE_PDF,
            outf
          )
        );
        ExporterXML xml(g->getInput(),true);
        xml.doExport(i);
        g->closeInput();
      }
    };
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //   PaymentSourceImpl
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    /// implementation of PaymentSource interface
    class PaymentSourceImpl : public PaymentSource
    {
      unsigned long number; ///< number of source advance invoice
      Money price; ///< money that come from this advance invoice  
      Money credit; ///< credit remaining on this advance invoice 
     public:
      /// init content from sql result (ignore first column)
      PaymentSourceImpl(DB *db, unsigned l) : 
        number(atol(db->GetFieldValue(l,1))),
        price(STR_TO_MONEY(db->GetFieldValue(l,2))),
        credit(STR_TO_MONEY(db->GetFieldValue(l,3)))
      {}
      virtual unsigned long getNumber() const
      {
        return number;
      }
      virtual Money getPrice() const
      {
        return price;
      }
      virtual Money getCredit() const
      {
        return credit;
      }
    };
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //   PaymentActionImpl
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    /// implementation of PaymentAction interface
    class PaymentActionImpl : public PaymentAction
    {
      std::string objectName; ///< name of object affected by payment action
      boost::posix_time::ptime actionTime; ///< time of payment action
      boost::gregorian::date exDate; ///< exdate of domain 
      PaymentActionType action; ///< type of action that is subject of payment
      unsigned unitsCount; ///< number of months to expiration of domain
      Money pricePerUnit; ///< copy of price from price list
      Money price; ///< summarized price for all units
     public:
      /// init content from sql result (ignore first column)
      PaymentActionImpl(DB *db, unsigned l) :
        objectName(db->GetFieldValue(l,1)),
        actionTime(MAKE_TIME(l,2)),
        exDate(MAKE_DATE(l,3)),
        action(
          atoi(db->GetFieldValue(l,4)) == 1 ? 
          PAT_CREATE_DOMAIN :
          PAT_RENEW_DOMAIN
        ),
        unitsCount(atoi(db->GetFieldValue(l,5))),
        pricePerUnit(STR_TO_MONEY(db->GetFieldValue(l,6))),
        price(STR_TO_MONEY(db->GetFieldValue(l,7)))
      {}      
      virtual const std::string& getObjectName() const
      {
        return objectName;
      }
      virtual boost::posix_time::ptime getActionTime() const
      {
        return actionTime;
      }
      virtual boost::gregorian::date getExDate() const
      {
        return exDate;
      }
      virtual PaymentActionType getAction() const
      {
        return action;
      }
      virtual unsigned getUnitsCount() const
      {
        return unitsCount;
      }
      virtual Money getPricePerUnit() const
      {
        return pricePerUnit;
      }
      virtual Money getPrice() const
      {
        return price;
      }
    };
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //   InvoiceImpl
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    /// implementation of interface Invoice
    class InvoiceImpl : public Invoice
    {
      TID id;
      TID zone;
      ptime crTime;
      boost::gregorian::date taxDate;
      time_period accountPeriod;
      Type type;
      unsigned long number;
      TID registrar;
      Money credit;
      Money price;
      short vatRate;
      Money total;
      Money totalVAT;
      std::string varSymbol;
      SubjectImpl client;
      static SubjectImpl supplier;
      std::vector<PaymentSourceImpl *> sources;
      std::vector<PaymentActionImpl *> actions;
      void clearLists()
      {
        for (unsigned i=0; i<sources.size(); i++) delete sources[i];
        for (unsigned i=0; i<actions.size(); i++) delete actions[i];
      }
     public:
      const Subject* getClient() const { return &client; }
      const Subject* getSupplier() const { return &supplier; }
      TID getId() const { return id; }
      TID getZone() const { return zone; }
      ptime getCrTime() const { return crTime; }
      boost::gregorian::date getTaxDate() const { return taxDate; }
      time_period getAccountPeriod() const
      { return accountPeriod; }
      Type getType() const { return type; }
      unsigned long getNumber() const { return number; }
      TID getRegistrar() const { return registrar; }
      Money getCredit() const { return credit; }
      Money getPrice() const { return price; }
      short getVatRate() const { return vatRate; }
      Money getTotal() const { return total; }
      Money getTotalVAT() const { return totalVAT; }
      const std::string& getVarSymbol() const { return varSymbol; }
      unsigned getSourceCount() const { return sources.size(); }
      const PaymentSource *getSource(unsigned idx) const
      {
        return idx>=sources.size() ? NULL : sources[idx];
      } 
      unsigned getActionCount() const { return actions.size(); }
      const PaymentAction *getAction(unsigned idx) const
      {
        return idx>=actions.size() ? NULL : actions[idx];
      }
      /// initialize invoice from result set=db with row=l 
      InvoiceImpl(DB *db, unsigned l) :
        id(STR_TO_ID(db->GetFieldValue(l,0))),
        zone(STR_TO_ID(db->GetFieldValue(l,1))),
        crTime(MAKE_TIME(l,2)),
        taxDate(MAKE_DATE(l,3)),
        accountPeriod(MAKE_TIME_NEG(l,4),MAKE_TIME_POS(l,5)),
        type(atoi(db->GetFieldValue(l,6)) == 0 ? IT_DEPOSIT : IT_ACCOUNT),
        number(atol(db->GetFieldValue(l,7))),
        registrar(STR_TO_ID(db->GetFieldValue(l,8))),
        credit(STR_TO_MONEY(db->GetFieldValue(l,9))),
        price(STR_TO_MONEY(db->GetFieldValue(l,10))),
        vatRate(atoi(db->GetFieldValue(l,11))),
        total(STR_TO_MONEY(db->GetFieldValue(l,12))),
        totalVAT(STR_TO_MONEY(db->GetFieldValue(l,13))),
        varSymbol(db->GetFieldValue(l,20)),
        client(
          db->GetFieldValue(l,14),
          "", // fullname is empty
          db->GetFieldValue(l,15),
          db->GetFieldValue(l,16),
          db->GetFieldValue(l,17),
          db->GetFieldValue(l,18),
          db->GetFieldValue(l,19),
          "", // registration is empty
          "", // reclamation is empty
          "", // url is empty
          "" // email is empty
        )
      {
      }
      ~InvoiceImpl()
      {
        clearLists();
      }
      /// export invoice using given exporter
      void doExport(Exporter *exp) const
      {
        exp->doExport(const_cast<InvoiceImpl*>(this));
      }
      /// test function for find algorithm
      bool hasId(TID id) const
      {
        return this->id == id;
      }
      /// initialize list of actions from sql result
      void addAction(DB *db, unsigned row)
      {
        actions.push_back(new PaymentActionImpl(db,row));
      }
      /// initialize list of sources from sql result
      void addSource(DB *db, unsigned row)
      {
        sources.push_back(new PaymentSourceImpl(db,row));        
      }
    };
    // TODO: should be initalized somewhere else
    /// static supplier in every invoice
    SubjectImpl InvoiceImpl::supplier(
      "CZ.NIC, z.s.p.o.",
      "CZ.NIC, zájmové sdružení právnických osob",
      "Americká 23",
      "Praha 2",
      "120 00",
      "67985726",
      "CZ67985726",
      "SpZ: odb. občanskopr. agend Magist. hl. m. Prahy, č. ZS/30/3/98",
      "CZ.NIC, z.s.p.o., Americká 23, 120 00 Praha 2",
      "www.nic.cz",
      "podpora@nic.cz"
    );
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //   InvoiceListImpl
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    /// implementation of interface InvoiceList
    class InvoiceListImpl : public InvoiceList
    {
      typedef std::vector<InvoiceImpl *> InvoicesType; ///< invoice list type
      InvoicesType invoices; ///< list of invoices for given filter
      DB *db; ///< database connection for querying
      TID idFilter; ///< filter for invoice id 
      TID registrarFilter; ///< filter for registrar recieving invoice 
      TID zoneFilter; ///< filter for id of associated zone
      unsigned typeFilter; ///< filter for invoice type (advance or normal)
      std::string varSymbolFilter; ///< filter for variable symbol of payment
      std::string numberFilter; ///< filter for invoice number
      time_period crDateFilter; ///< filter for crDate
      time_period taxDateFilter; ///< filter for taxDate
      ArchiveFilter archiveFilter; ///< filter for existance of archived file
      TID objectFilter; ///< filter for attached object
      std::string advanceNumberFilter; ///< filter for source advance invoice
      /// find invoice in list by id
      InvoiceImpl *findById(TID id) const
      {
        InvoicesType::const_iterator i = find_if(
          invoices.begin(), invoices.end(),
          std::bind2nd(std::mem_fun(&InvoiceImpl::hasId),id)
        );
        return i == invoices.end() ? NULL : *i;
      }      
     public:
      InvoiceListImpl(DB *_db) : db(_db), 
        idFilter(0), registrarFilter(0), zoneFilter(0), typeFilter(0), 
        crDateFilter(ptime(neg_infin),ptime(pos_infin)),
        taxDateFilter(ptime(neg_infin),ptime(pos_infin)),
        archiveFilter(AF_IGNORE), objectFilter(0)
      {}
      ~InvoiceListImpl()
      {
        clearList();
      }
      void clearList()
      {
        for (unsigned i=0; i<invoices.size(); i++)
          delete invoices[i];
        invoices.clear();
      }
      void clearFilter()
      {
        idFilter = 0;
        registrarFilter = 0; 
        zoneFilter = 0;
        typeFilter = 0;
        varSymbolFilter = "";
        numberFilter = "";
        crDateFilter = time_period(ptime(neg_infin),ptime(pos_infin));
        taxDateFilter = time_period(ptime(neg_infin),ptime(pos_infin));
        archiveFilter = AF_IGNORE;
        objectFilter = 0;
      }
      virtual void setIdFilter(TID id)
      {
        idFilter = id;
      }
      virtual void setRegistrarFilter(TID registrar)
      {
        registrarFilter = registrar;
      }
      virtual void setZoneFilter(TID zone)
      {
        zoneFilter = zone;
      }
      virtual void setTypeFilter(unsigned type)
      {
        typeFilter = type;
      }
      virtual void setVarSymbolFilter(const std::string& varSymbol)
      {
        varSymbolFilter = varSymbol;
      }
      virtual void setNumberFilter(const std::string& number)
      {
        numberFilter = number;
      }
      virtual void setCrDateFilter(const time_period& crDatePeriod)
      {
        crDateFilter = crDatePeriod;
      }
      virtual void setTaxDateFilter(const time_period& taxDatePeriod)
      {
        taxDateFilter = taxDatePeriod;
      }
      virtual void setArchivedFilter(ArchiveFilter archive)
      {
        archiveFilter = archive;
      }
      virtual void setObjectFilter(TID object)
      {
        objectFilter = object;
      }
      virtual void setAdvanceNumberFilter(const std::string& number)
      {
        advanceNumberFilter = number;
      }
      virtual void reload() throw (SQL_ERROR)
      {
        clearList();
        std::stringstream sql;        
        // id that conform to filter will be stored in temporary table
        // sql is contructed from two sections 'from' and 'where'
        // that are pasted into final 'sql' stream 
        sql << "SELECT i.id INTO TEMPORARY tmp_invoice_filter_result ";
        std::stringstream from;
        from << "FROM invoice i ";            
        std::stringstream where;
        where << "WHERE 1=1 "; // must be for the case of empty filter
        // process individual filters 
        SQL_ID_FILTER(where,"i.id",idFilter);
        SQL_ID_FILTER(where,"i.registrarid",registrarFilter);
        SQL_ID_FILTER(where,"i.zone",zoneFilter);
        if (typeFilter) {
          from << ", invoice_prefixes ip ";
          where << "AND i.prefix_type=ip.id ";
          SQL_ID_FILTER_FILL(where,"ip.typ",typeFilter);
        }
        if (!varSymbolFilter.empty()) {
          from << ", registrar r ";
          where << "AND i.registrarid=r.id ";
          SQL_WILDCARD_FILTER_FILL(where,"r.varsymb",varSymbolFilter);
        }
        SQL_WILDCARD_FILTER(where,"i.prefix",numberFilter);
        SQL_DATE_FILTER(where,"i.crdate",crDateFilter);
        SQL_TIME_FILTER(where,"i.taxdate",taxDateFilter);
        switch (archiveFilter) {
          case AF_IGNORE: break;
          case AF_SET: where << "AND NOT(i.file ISNULL) "; break;
          case AF_UNSET: where << "AND i.file ISNULL "; break;
        }
        if (objectFilter) {
          from << ", invoice_object_registry ior ";
          where << "AND i.id=ior.invoiceid ";
          SQL_ID_FILTER_FILL(where,"ior.objectid",objectFilter);
        }
        // complete sql end do the query
        sql << from.rdbuf() << where.rdbuf();
        if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
        // initialize list of invoices using temporary table 
        if (!db->ExecSelect(
          "SELECT "
          " i.id, i.zone, i.crdate, i.taxdate, ig.fromdate, "
          " ig.todate, ip.typ, i.prefix, i.registrarid, i.credit*100, "
          " i.price*100, i.vat, i.total*100, i.totalvat*100, "
          " r.organization, r.street1, "
          " r.city, r.postalcode, r.ico, r.dic, r.varsymb "
          "FROM "
          " tmp_invoice_filter_result it, registrar r, "
          " invoice_prefix ip, invoice i "
          " LEFT JOIN invoice_generation ig ON (i.id=ig.invoiceid) "
          "WHERE "
          " it.id=i.id AND i.registrarid=r.id AND ip.id=i.prefix_type "
        )) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++)         
          invoices.push_back(new InvoiceImpl(db,i));
        // append list of actions to all selected invoices
        if (!db->ExecSelect(
          "SELECT "
          " it.id, o.name, ior.crdate, ior.exdate, ior.operation, ior.period, "
          " CASE "
          "  WHEN ior.period=0 THEN 0 "
          "  ELSE SUM(iorpm.price)/ior.period END, "
          " SUM(iorpm.price) "
          "FROM "
          " tmp_invoice_filter_result it, "
          " invoice_object_registry ior, object_registry o, "
          " invoice_object_registry_price_map iorpm "
          "WHERE "
          " it.id=ior.invoiceid AND ior.objectid=o.id "
          " AND ior.id=iorpm.id "
          "GROUP BY "
          " it.id, o.name, ior.crdate, ior.exdate, ior.operation, ior.period "
        )) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          InvoiceImpl *inv = findById(STR_TO_ID(db->GetFieldValue(i,0)));
          if (inv) inv->addAction(db,i);
          else {
            // TODO: log error - some database problem 
          }
        }
        // append list of sources to all selected invoices
        if (!db->ExecSelect(
          "SELECT "
          " it.id, sri.prefix, SUM(iorpm.price) "
          "FROM "
          " tmp_invoice_filter_result it, "
          " invoice_object_registry ior, "
          " invoice_object_registry_price_map iorpm, invoice sri "
          "WHERE "
          " it.id=ior.invoiceid AND ior.id=iorpm.id "
          " AND iorpm.invoiceid=sri.id "
          "GROUP BY "
          " it.id, iorpm.id, sri.prefix "
        )) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          InvoiceImpl *inv = findById(STR_TO_ID(db->GetFieldValue(i,0)));
          if (inv) inv->addSource(db,i);
          else {
            // TODO: log error - some database problem 
          }
        }
        // delete temporary table
        if (!db->ExecSQL("DROP TABLE tmp_invoice_filter_result "))
          throw SQL_ERROR();
      }
      /// export all invoices on the list using given exporter
      void doExport(Exporter *exp) const
      {
        for_each(
          invoices.begin(),invoices.end(),
          std::bind2nd(std::mem_fun(&InvoiceImpl::doExport),exp)
        );
      }
      /// return count of invoices in list 
      virtual unsigned long getCount() const
      {
        return invoices.size();
      }
      virtual void exportXML(std::ostream& out) const
      {
        out << "<?xml version='1.0' encoding='utf-8'?>";        
        ExporterXML xml(out,false);
        if (getCount()!=1) out << TAGSTART(list);
        doExport(&xml);
        if (getCount()!=1) out << TAGEND(list);
      }           
    }; // InvoiceListImpl 
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    //   ManagerImpl
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    /// implementation for Manager interface
    class ManagerImpl : public Manager
    {
      DB *db;
      Document::Manager *docman;
     public:
      ManagerImpl(DB *_db, Document::Manager *_docman) 
        : db(_db), docman(_docman)
      {}
      /// find unarchived invoices and archive then in PDF format
      void archiveInvoices() const
      {
        ExporterPDF pdf(docman);
        InvoiceListImpl l(db);
        l.reload();
        l.doExport(&pdf);
      }
      /// create empty list of invoices      
      virtual InvoiceList* createList() const
      {
        return new InvoiceListImpl(db);
      }
    }; // ManagerImpl
    Manager *Manager::create(DB *db, Document::Manager *docman)
    {
      return new ManagerImpl(db,docman);
    }    
  }; // Invoicing
}; // Register
