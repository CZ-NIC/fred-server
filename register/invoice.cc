#include "dbsql.h" 
#include "invoice.h"
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
    /// exporter interface 
    class Exporter
    {
     public:
      virtual ~Exporter() {}
      virtual void doExport(Invoice *) = 0;
    };
    #define TAGSTART(tag) "<"#tag">"
    #define TAGEND(tag) "</"#tag">"
    #define TAG(tag,f) TAGSTART(tag) << f << TAGEND(tag)
    #define OUTMONEY(f) (f)/100 << "." << \
                        std::setfill('0') << std::setw(2) << (f)%100
    // exporter that export xml of invoice into given stream
    class ExporterXML : public Exporter
    {
      std::ostream& out;
     public:
      ExporterXML(std::ostream& _out) : out(_out)
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
          std::locale(),
          new boost::gregorian::date_facet("%d.%m.%Y")
        ));
        // generate invoice xml
        out << "<?xml version='1.0' encoding='utf-8'?>"
            << TAGSTART(invoice)
            << TAGSTART(client);
        doExport(i->getClient());
        out << TAGEND(client)
            << TAGSTART(supplier);
        doExport(i->getSupplier());
        out << TAGEND(supplier)
            << TAGSTART(payment)
            << TAG(invoice_number,i->getNumber())
            << TAG(invoice_date,i->getCrTime().date())
            << TAG(payment_date,i->getCrTime().date())
            << TAG(tax_point,i->getTaxDate())
            << TAG(vs,i->getVarSymbol())
/*            
            << TAG(ks,i->getConstSymbol())
            << TAG(payment_method,"bankovním převodem")
            << TAGSTART(bank)
            << TAG(name,"ČSOB, pobočka Praha")
            << TAG(address,"Perlova 5, Praha")
            << TAG(account,"152903575/0300")
            << TAG(iban,"CZ56 0300 0000 0001 5290 3575")
            << TAGEND(bank)
 */
            << TAG(anotation,
                   "Fakturujeme Vám přijetí zálohy za služby "
                   "(Smlouva o spolupráci při registracích doménových "
                   "jmen ENUM)."
               )
/*
            << TAG(note,
                   "DPH byla vypořádána na zálohových daňových dokladech."
               )
 */
            << TAG(issue_person,"Vystaveno fakturačním systémem CZ.NIC")
            << TAGEND(payment)
            << TAGSTART(delivery)
            << TAGSTART(entry)
            << TAG(vatperc,i->getVatRate())
            << TAG(basetax,OUTMONEY(i->getTotal()))
            << TAG(vat,OUTMONEY(i->getTotalVAT()))
            << TAG(total,OUTMONEY(i->getPrice()))
            << TAGEND(entry)
            << TAGEND(delivery)
            << TAGSTART(sumarize)
            << TAG(total,OUTMONEY(i->getPrice()))
            << TAG(paid,OUTMONEY(i->getPrice()))
            << TAG(to_be_paid,OUTMONEY(0))
            << TAGEND(sumarize)
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
    class ExporterPDF : public Exporter
    {
     public:
      ExporterPDF() {}
      virtual void doExport(Invoice *i)
      {
        std::stringstream filename;
        filename << "/tmp/f" << i->getNumber() << ".xml";
        std::stringstream filenamePDF;
        filenamePDF << "/tmp/f" << i->getNumber() << ".pdf";
        {
          std::ofstream f(filename.str().c_str());
          ExporterXML xml(f);
          xml.doExport(i);
        }
        std::stringstream cmd;
        cmd << "xsltproc "
            << "/home/jara/enum/fred2pdf/trunk/templates/invoice.xsl "
            << filename.str()
            << " | python /home/jara/enum/fred2pdf/trunk/doc2pdf.py > "
            << filenamePDF.str();
        system(cmd.str().c_str());
      }
    };
    class InvoiceImpl : public Invoice
    {
      TID id;
      TID zone;
      boost::posix_time::ptime crTime;
      boost::gregorian::date taxDate;
      boost::posix_time::time_period accountPeriod;
      Type type;
      unsigned long number;
      TID registrar;
      Money credit;
      Money price;
      short vatRate;
      Money total;
      Money totalVAT;
      std::string constSymbol;
      std::string varSymbol;
      SubjectImpl client;
      static SubjectImpl supplier; 
     public:
      const Subject* getClient() const { return &client; }
      const Subject* getSupplier() const { return &supplier; }
      TID getId() const { return id; }
      TID getZone() const { return zone; }
      boost::posix_time::ptime getCrTime() const { return crTime; }
      boost::gregorian::date getTaxDate() const { return taxDate; }
      boost::posix_time::time_period getAccountPeriod() const
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
      const std::string& getConstSymbol() const { return constSymbol; }
      InvoiceImpl(DB *db, unsigned l) :
        id(STR_TO_ID(db->GetFieldValue(l,0))),
        zone(STR_TO_ID(db->GetFieldValue(l,1))),
        crTime(MAKE_TIME(l,2)),
        taxDate(MAKE_DATE(l,3)),
        accountPeriod(MAKE_TIME_NEG(l,4),MAKE_TIME_POS(l,5)),
        type(atoi(db->GetFieldValue(l,6)) == 0 ? IT_DEPOSIT : IT_ACCOUNT),
        number(atol(db->GetFieldValue(l,7))),
        registrar(STR_TO_ID(db->GetFieldValue(l,8))),
        credit(atol(db->GetFieldValue(l,9))),
        price(atol(db->GetFieldValue(l,10))),
        vatRate(atol(db->GetFieldValue(l,11))),
        total(atol(db->GetFieldValue(l,12))),
        totalVAT(atol(db->GetFieldValue(l,13))),
        constSymbol("308"),
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
      /// export invoice using given exporter
      void doExport(Exporter *exp) const
      {
        exp->doExport(const_cast<InvoiceImpl*>(this));
      }
    };
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
    class InvoiceListImpl : public InvoiceList
    {
      typedef std::vector<InvoiceImpl *> InvoicesType; ///< invoice list type
      InvoicesType invoices; ///< list of invoices for given filter
      DB *db;
      TID idFilter;
     public:
      InvoiceListImpl(DB *_db) : db(_db), idFilter(0)
      {}
      void clearFilter()
      {
        idFilter = 0;
      }
      void setIdFilter(TID _idFilter)
      {
        idFilter = _idFilter;
      }
      /// load from database list of invoices according to selected filter
      void reload() throw (SQL_ERROR)
      {
        for (unsigned i=0; i<invoices.size(); i++)
          delete invoices[i];
        invoices.clear();
        std::stringstream sql;
        sql << "SELECT i.id, i.zone, i.crdate, i.taxdate, i.fromdate, "
            << "i.todate, i.typ, i.prefix, i.registrarid, i.credit*100, "
            << "i.price*100, i.vat, i.total*100, i.totalvat*100, "
            << "r.organization, r.street1, "
            << "r.city, r.postalcode, r.ico, r.dic, r.varsymb "
            << "FROM invoice i, registrar r "
            << "WHERE i.registrarid=r.id ";
        if(idFilter) sql << "AND i.id=" << idFilter;
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++)
          invoices.push_back(new InvoiceImpl(db,i));
      }
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
      DB *db;
     public:
      ManagerImpl(DB *_db) : db(_db)
      {}
      /// find unarchived invoices and archive then in PDF format
      void archiveInvoices() const
      {
        ExporterPDF pdf;
        InvoiceListImpl l(db);
        l.reload();
        l.doExport(&pdf);
      }
    }; // ManagerImpl
    Manager *Manager::create(DB *db)
    {
      return new ManagerImpl(db);
    }    
  }; // Invoicing
}; // Register
