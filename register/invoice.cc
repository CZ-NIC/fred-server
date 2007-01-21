#include "dbsql.h" 
#include "invoice.h"
#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>

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
        out << "<?xml version='1.0' encoding='utf-8'?>"
            << TAGSTART(invoice)
            << TAG(client,doExport(i->getClient()))
            << TAG(supplier,doExport(i->getSupplier()))
            << TAGEND(invoice);
/*

<payment>
    <invoice_number>2406000590</invoice_number>
    <invoice_date>3.10.2006</invoice_date>
    <payment_date>3.10.2006</payment_date>
    <tax_point>2.10.2006</tax_point>
    <vs>45021295</vs>
    <ks>308</ks>
    <payment_method>bankovním převodem</payment_method>
    <bank>
        <name>ČSOB, pobočka Praha</name>
        <address>Perlova 5, Praha</address>
        <account>152903575/0300</account>
        <iban>CZ56 0300 0000 0001 5290 3575</iban>
    </bank>
    <anotation>
    Fakturujeme Vám přijetí zálohy za služby (smlouva o statusu Registrátora).
    </anotation>
    <note>DPH byla vypořádána na zálohových daňových dokladech.</note>
    <issue_person>Zuzana Durajová</issue_person>
</payment>

<delivery>
    <entry>
        <vatperc>0</vatperc>
        <basetax>0</basetax>
        <vat>0</vat>
        <total>0</total>
    </entry>
    <entry>
        <vatperc>5</vatperc>
        <basetax>0</basetax>
        <vat>0</vat>
        <total>0</total>
    </entry>
    <entry>
        <vatperc>19</vatperc>
        <basetax>42015</basetax>
        <vat>7985</vat>
        <total>50000</total>
    </entry>
</delivery>

<sumarize>
    <total>50000</total>
    <paid>50000</paid>
    <to_be_paid>0</to_be_paid>
</sumarize>


<appendix>

<items>
    <item>
        <subject>gaad.cz</subject>
        <code>RUDR</code>
        <date>13.11.2007</date>
        <count>1</count>
        <vatperc>0</vatperc>
        <price>300</price>
        <total>300</total>
    </item>
    <item>
        <subject>jindrizska.cz</subject>
        <code>RUDR</code>
        <date>3.11.2007</date>
        <count>2</count>
        <vatperc>0</vatperc>
        <price>400</price>
        <total>800</total>
    </item>
    <item>
        <subject>snih.cz</subject>
        <code>RUDR</code>
        <date>10.10.2007</date>
        <count>1</count>
        <vatperc>0</vatperc>
        <price>500</price>
        <total>500</total>
    </item>
</items>

<sumarize_items>
    <total>1500</total>
</sumarize_items>

</appendix>

</invoice>";
*/
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
      SubjectImpl client;
      static SubjectImpl supplier; 
     public:
      const Subject* getClient() const
      {
        return &client;
      }
      const Subject* getSupplier() const
      {
        return &supplier;
      }
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
      InvoiceImpl(DB *db, unsigned l) :
        accountPeriod(ptime(neg_infin),ptime(pos_infin)),
        client(
         db->GetFieldValue(l,14),"",db->GetFieldValue(l,15),
         db->GetFieldValue(l,16),"",db->GetFieldValue(l,17),
         db->GetFieldValue(l,18),
         "","","",""
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
      "fakt@nic.cz"
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
            << "i.todate, i.typ, i.prefix, i.registrarid, i.credit, i.price, "
            << "i.vat, i.total, i.totalvat, r.organization, r.street1, "
            << "r.city, r.ico, r.vat "
            << "FROM invoice i, registrar r "
            << "WHERE i.registrar=r.id ";
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
        ExporterXML xml(std::cout);
        InvoiceListImpl l(db);
        l.reload();
        l.doExport(&xml);
      }
    }; // ManagerImpl
    Manager *Manager::create(DB *db)
    {
      return new ManagerImpl(db);
    }    
  }; // Invoicing
}; // Register
