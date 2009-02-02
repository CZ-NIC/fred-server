/*
 *  Copyright (C) 2007, 2008, 2009  CZ.NIC, z.s.p.o.
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

#include "invoice.h"

#include <algorithm>
#include <boost/utility.hpp>

#include "common_impl.h"
#include "log/logger.h"

#include "file.h"
#include <cmath>
#include "documents.h"

namespace Register {
namespace Invoicing {

#define INVOICE_PDF_FILE_TYPE 1 
#define INVOICE_XML_FILE_TYPE 2 

#define TAGSTART(tag) "<"#tag">"
#define TAGEND(tag) "</"#tag">"
#define TAG(tag,f) TAGSTART(tag) \
    << "<![CDATA[" << f << "]]>" << TAGEND(tag)
#define OUTMONEY(f) (f)/100 << "." << \
    std::setfill('0') << std::setw(2) << abs(f)%100

#define transformString(str)    \
    ((str.empty()) ? "NULL" : "'" + str + "'")
#define transformId(id)         \
    ((id.to_string().compare("0") == 0) ? "NULL" : id.to_string())

#define ERROR(values)                           \
    LOGGER(PACKAGE).error(values);              \
    addError(values);

std::string
Type2Str(Type type)
{
    switch (type) {
        case IT_DEPOSIT:    return "Deposit";
        case IT_ACCOUNT:    return "Account";
        default:            return "TYPE UNKNOWN";
    }
}

int
Type2SqlType(Type type)
{
    switch (type) {
        case IT_DEPOSIT:    return 0;
        case IT_ACCOUNT:    return 1;
        default:            return -1;
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

/*! return payment action type as defined in database
 * (table ``enum_operation''
 */
int
PaymentActionType2SqlType(PaymentActionType type)
{
    switch (type) {
        case PAT_CREATE_DOMAIN: return 1;
        case PAT_RENEW_DOMAIN:  return 2;
        default:                return -1;
    }
}

class SubjectImpl:
    public Subject {
private:
    Database::ID m_id;
    std::string m_handle;
    std::string m_name;
    std::string m_fullname;
    std::string m_street;
    std::string m_city;
    std::string m_zip;
    std::string m_country;
    std::string m_ico;
    std::string m_vatNumber;
    std::string m_registration;
    std::string m_reclamation;
    std::string m_email;
    std::string m_url;
    std::string m_phone;
    std::string m_fax;
    bool m_vatApply;
public:
    SubjectImpl()
    { }
    SubjectImpl(Database::ID _id, const std::string& _handle,
            const std::string& _name, const std::string& _fullname, 
            const std::string& _street, const std::string& _city, 
            const std::string& _zip, const std::string& _country, 
            const std::string& _ico, const std::string& _vatNumber, 
            const std::string& _registration, const std::string& _reclamation, 
            const std::string& _email, const std::string& _url, 
            const std::string& _phone, const std::string& _fax, bool _vatApply):
        m_id(_id), m_handle(_handle), m_name(_name), m_fullname(_fullname),
        m_street(_street), m_city(_city), m_zip(_zip), m_country(_country),
        m_ico(_ico), m_vatNumber(_vatNumber), m_registration(_registration),
        m_reclamation(_reclamation), m_email(_email), m_url(_url), m_phone(_phone),
        m_fax(_fax), m_vatApply(_vatApply)
    { }
    Database::ID getId() const 
    {
        return m_id;
    }
    const std::string& getHandle() const 
    {
        return m_handle;
    }
    const std::string& getName() const 
    {
        return m_name;
    }
    const std::string& getFullname() const 
    {
        return m_fullname;
    }
    const std::string& getStreet() const 
    {
        return m_street;
    }
    const std::string& getCity() const 
    {
        return m_city;
    }
    const std::string& getZip() const 
    {
        return m_zip;
    }
    const std::string& getCountry() const 
    {
        return m_country;
    }
    const std::string& getICO() const 
    {
        return m_ico;
    }
    const std::string& getVatNumber() const 
    {
        return m_vatNumber;
    }
    bool getVatApply() const 
    {
        return m_vatApply;
    }
    const std::string& getRegistration() const 
    {
        return m_registration;
    }
    const std::string& getReclamation() const 
    {
        return m_reclamation;
    }
    const std::string& getEmail() const 
    {
        return m_email;
    }
    const std::string& getURL() const 
    {
        return m_url;
    }
    const std::string& getPhone() const 
    {
        return m_phone;
    }
    const std::string& getFax() const 
    {
        return m_fax;
    }
}; // class SubjectImpl;

class PaymentImpl:
    virtual public Payment {
private:
    Database::Money     m_price;
    unsigned int        m_vatRate;
    Database::Money     m_vat;
public:
    PaymentImpl(const PaymentImpl *sec):
        m_price(sec->m_price), m_vatRate(sec->m_vatRate), m_vat(sec->m_vat)
    { }
    PaymentImpl(Database::Money price, unsigned int vatRate, Database::Money vat):
        m_price(price), m_vatRate(vatRate), m_vat(vat)
    { }
    virtual Database::Money getPrice() const
    {
        return m_price;
    }
    virtual unsigned int getVatRate() const
    {
        return m_vatRate;
    }
    virtual Database::Money getVat() const
    {
        return m_vat;
    }
    virtual Database::Money getPriceWithVat() const
    {
        return m_price + m_vat;
    }
    bool operator==(unsigned vatRate) const
    {
        return m_vatRate == vatRate;
    }
    void add(const PaymentImpl *sec)
    {
        m_price = m_price + sec->m_price;
        m_vat = m_vat + sec->m_vat;
    }
}; // class PaymentImpl;

class PaymentSourceImpl:
    public PaymentImpl,
    virtual public PaymentSource {
private:
    Database::ID        m_id;
    unsigned long long  m_number;
    Database::Money     m_credit;
    Database::Money     m_totalPrice;
    Database::Money     m_totalVat;
    Database::DateTime  m_crTime;
public:
    PaymentSourceImpl(Database::Money price, unsigned vatRate,
            Database::Money vat, unsigned long long number,
            Database::Money credit, Database::ID id, Database::Money totalPrice,
            Database::Money totalVat, Database::DateTime crTime):
        PaymentImpl(price, vatRate, vat),
        m_id(id), m_number(number), m_credit(credit), m_totalPrice(totalPrice),
        m_totalVat(totalVat), m_crTime(crTime)
    { }

    virtual Database::ID getId() const
    {
        return m_id;
    }

    virtual unsigned long long getNumber() const
    {
        return m_number;
    }

    virtual Database::Money getCredit() const
    {
        return m_credit;
    }

    virtual Database::Money getTotalPrice() const
    {
        return m_totalPrice;
    }

    virtual Database::Money getTotalVat() const
    {
        return m_totalVat;
    }

    virtual Database::Money getTotalPriceWithVat() const
    {
        return m_totalPrice + m_totalVat;
    }

    virtual Database::DateTime getCrTime() const
    {
        return m_crTime;
    }
}; // class PaymentSourceImpl;

class PaymentActionImpl:
    public PaymentImpl,
    virtual public PaymentAction {
private:
    std::string         m_objectName;
    Database::DateTime  m_actionTime;
    Database::Date      m_exDate;
    PaymentActionType   m_action;
    unsigned int        m_unitsCount;
    Database::Money     m_pricePerUnit;
    Database::ID        m_objectId;
public:
    PaymentActionImpl(Database::Money price, unsigned int vatRate, 
            Database::Money vat, std::string &objectName, 
            Database::DateTime actionTime, Database::Date exDate,
            PaymentActionType action, unsigned int unitsCount,
            Database::Money pricePerUnit, Database::ID objectId):
        PaymentImpl(price, vatRate, vat),
        m_objectName(objectName), m_actionTime(actionTime), m_exDate(exDate),
        m_action(action), m_unitsCount(unitsCount),
        m_pricePerUnit(pricePerUnit), m_objectId(objectId)
    { }
    virtual Database::ID getObjectId() const 
    {
        return m_objectId;
    }
    virtual const std::string &getObjectName() const 
    {
        return m_objectName;
    }
    virtual Database::DateTime getActionTime() const 
    {
        return m_actionTime;
    }
    virtual Database::Date getExDate() const
    {
        return m_exDate;
    }
    virtual PaymentActionType getAction() const 
    {
        return m_action;
    }
    virtual unsigned int getUnitsCount() const 
    {
        return m_unitsCount;
    }
    virtual Database::Money getPricePerUnit() const 
    {
        return m_pricePerUnit;
    }
}; // class PaymentActionImpl;

class AnnualPartitioningImpl:
    public virtual AnnualPartitioning {
private:
    typedef std::map<unsigned int, Database::Money> RecordsType;
    typedef std::map<unsigned int, RecordsType> vatRatesRecordsType;

    RecordsType::const_iterator         m_i;
    vatRatesRecordsType::const_iterator m_j;
    vatRatesRecordsType                 m_records;
    Manager                             *m_manager;
    bool                                m_noVatRate;
public:
    AnnualPartitioningImpl(Manager *manager = NULL):
        m_manager(manager), m_noVatRate(true)
    { }

    void addAction(PaymentAction *action)
    {
        using namespace boost::gregorian;
        using namespace boost::posix_time;

        if (!action->getUnitsCount() || action->getExDate().is_special()) {
            return;
        }
        date lastdate = action->getExDate().get();
        int dir = (action->getPrice() > Database::Money(0)) ? 1 : -1;
        date firstdate = action->getExDate().get() - months(dir * action->getUnitsCount());
        if (dir) {
            date pom = lastdate;
            lastdate = firstdate;
            firstdate = lastdate;
            // std::swap(lastdate, firstdate);
        }
        Database::Money remains = action->getPrice();
        while (remains) {
            Database::Money part;
            unsigned int year = lastdate.year();
            if (year == firstdate.year()) {
                part = remains;
            } else {
                date newdate = date(year, 1, 1) - days(1);
                part = remains * (lastdate - newdate).days() /
                    (lastdate - firstdate).days();
                lastdate = newdate;
            }
            remains = remains - part;
            m_records[action->getVatRate()][year] =
                m_records[action->getVatRate()][year] + part;
        }
    } // AnnualPartitioningImpl::addAction();

    void resetIterator(unsigned int vatRate)
    {
        m_j = m_records.find(vatRate);
        if (m_j == m_records.end()) {
            m_noVatRate = true;
        } else {
            m_noVatRate = false;
            m_i = m_j->second.begin();
        }
    }
    
    bool end() const
    {
        return m_noVatRate || m_i == m_j->second.end();
    }

    void next()
    {
        m_i++;
    }

    unsigned int getYear() const
    {
        return end() ? 0 : m_i->first;
    }

    Database::Money getPrice() const
    {
        return end() ? Database::Money(0) : m_i->second;
    }

    unsigned int getVatRate() const
    {
        return end() ? 0 : m_j->first;
    }

    Database::Money getVat() const
    {
        return m_manager->countVat(getPrice(), getVatRate(), true);
    }

    Database::Money getPriceWithVat() const
    {
        return getPrice() + getVat();
    }
}; // class AnnualPartitioningImpl;

class InvoiceImpl:
    public Register::CommonObjectImpl,
    virtual public Invoice {
private:
    Database::ID            m_zone;
    std::string             m_zoneName;
    Database::DateTime      m_crTime;
    Database::Date          m_taxDate;
    Database::Date          m_toDate;
    Database::DateInterval  m_accountPeriod;
    Type                    m_type;
    unsigned long long      m_number;
    Database::ID            m_registrar;
    std::string             m_registrarName;
    Database::Money         m_credit;
    Database::Money         m_price;
    int                     m_vatRate;
    Database::Money         m_total;
    Database::Money         m_totalVAT;
    Database::ID            m_filePDF;
    Database::ID            m_fileXML;
    std::string             m_varSymbol;
    SubjectImpl             m_client;
    static SubjectImpl      m_supplier;
    std::string             m_filePDF_name;
    std::string             m_fileXML_name;
    Database::Connection    *m_conn;

    AnnualPartitioningImpl              m_annualPartitioning;
    Database::Manager       *m_dbMan;
    Register::File::File    *m_file1;
    Register::File::File    *m_file2;

    Manager                 *m_manager;

    std::vector<PaymentSourceImpl *>    m_sources;
    std::vector<PaymentActionImpl *>    m_actions;
    std::vector<PaymentImpl>            m_paid;

    Database::ID            m_invoicePrefixTypeId;
    bool                    m_storeFileFlag;

    void clearLists()
    {
        for (unsigned int i = 0; i < m_sources.size(); i++) {
            delete m_sources[i];
        }
        for (unsigned int i = 0; i < m_actions.size(); i++) {
            delete m_actions[i];
        }
    }
    std::vector<std::string>    m_errors;
    Database::Date              m_fromDate;
public:
    InvoiceImpl(Database::ID id, Database::ID zone, std::string zoneName,
            Database::DateTime crTime, Database::Date taxDate, 
            Database::DateInterval accountPeriod, Type type,
            unsigned long long number, Database::ID registrar,
            Database::Money credit, Database::Money price, int vatRate,
            Database::Money total, Database::Money totalVAT,
            Database::ID filePDF, Database::ID fileXML, std::string varSymbol,
            SubjectImpl &client, std::string filePDF_name, std::string fileXML_name,
            Database::Manager *dbMan, Database::Connection *conn, 
            Manager *manager):
        CommonObjectImpl(id),
        m_zone(zone), m_zoneName(zoneName), m_crTime(crTime), m_taxDate(taxDate),
        m_accountPeriod(accountPeriod), m_type(type), m_number(number),
        m_registrar(registrar), m_credit(credit), m_price(price), m_vatRate(vatRate),
        m_total(total), m_totalVAT(totalVAT), m_filePDF(filePDF), m_fileXML(fileXML),
        m_varSymbol(varSymbol), m_client(client), m_filePDF_name(filePDF_name), 
        m_fileXML_name(fileXML_name), m_conn(conn),
        m_annualPartitioning(manager), m_dbMan(dbMan), m_manager(manager),
        m_storeFileFlag(false)
    { }

    InvoiceImpl(Database::Manager *dbMan, Database::Connection *conn,
            Manager *manager):
        CommonObjectImpl(),
        m_number(0),
        m_vatRate(-1),
        m_conn(conn),
        m_dbMan(dbMan),
        m_manager(manager),
        m_storeFileFlag(false)
    { }

    ~InvoiceImpl()
    {
        clearLists();
    }

    Database::ID getZone() const
    {
        return m_zone;
    }
    const std::string &getZoneName() const
    {
        return m_zoneName;
    }
    Database::DateTime getCrTime() const
    {
        return m_crTime;
    }
    Database::Date getTaxDate() const
    {
        return m_taxDate;
    }
    Database::Date getToDate() const
    {
        return m_toDate;
    }
    Database::Date getFromDate() const
    {
        return m_fromDate;
    }
    Database::DateInterval getAccountPeriod() const
    {
        return m_accountPeriod;
    }
    Type getType() const
    {
        return m_type;
    }
    unsigned long long getNumber() const
    {
        return m_number;
    }
    Database::ID getRegistrar() const
    {
        return m_registrar;
    }
    std::string getRegistrarName() const
    {
        return m_registrarName;
    }
    Database::Money getCredit() const
    {
        return m_credit;
    }
    Database::Money getPrice() const
    {
        return m_price;
    }
    int getVatRate() const
    {
        return m_vatRate;
    }
    Database::Money getTotal() const
    {
        return m_total;
    }
    Database::Money getTotalVAT() const
    {
        return m_totalVAT;
    }
    const std::string& getVarSymbol() const
    {
        return m_varSymbol;
    }
    Database::ID getFilePDF() const
    {
        return m_filePDF;
    }
    Database::ID getFileXML() const
    {
        return m_fileXML;
    }
    std::string getFileNamePDF() const
    {
        return m_filePDF_name;
    }
    std::string getFileNameXML() const
    {
        return m_fileXML_name;
    }
    
    void addError(std::string str)
    {
        m_errors.push_back(str);
    }
    void addError(boost::format frmt)
    {
        m_errors.push_back(frmt.str());
    }

    virtual std::string getLastError() const
    {
        return (m_errors.size() == 0) ? "" : m_errors.back();
    }
    virtual std::vector<std::string> getErrors() const 
    {
        return m_errors;
    }

    void addSource(Database::Row::Iterator& _col, Manager *manager)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "addSource(Database::Row::Iterator &, Manager)");
        Database::Money price       = *_col;
        unsigned vat_rate           = *(++_col);
        unsigned long long number   = *(++_col);  
        Database::Money credit      = *(++_col);
        Database::ID id             = *(++_col);
        Database::Money total_price = *(++_col);
        Database::Money total_vat   = *(++_col);
        Database::DateTime crtime   = *(++_col);

        PaymentSourceImpl *new_source =
            new PaymentSourceImpl(price, vat_rate, 
                    manager->countVat(price, vat_rate, true), number, credit, 
                    id, total_price, total_vat, crtime);
        m_sources.push_back(new_source);

        // init vat groups, if vat rate exists, add it, otherwise create new
        std::vector<PaymentImpl>::iterator i =
            find(m_paid.begin(), m_paid.end(), new_source->getVatRate() );
        if (i != m_paid.end()) {
            i->add(new_source);
        } else {
            m_paid.push_back(PaymentImpl(new_source));    
        }
    } // InvoiceImpl::addSource()
    void addSource(PaymentSourceImpl *source)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "addSource(PaymentSourceImpl *)");
        m_sources.push_back(source);

        std::vector<PaymentImpl>::iterator it =
            find(m_paid.begin(), m_paid.end(), source->getVatRate());
        if (it != m_paid.end()) {
            it->add(source);
        } else {
            m_paid.push_back(PaymentImpl(source));
        }
    } // InvoiceImpl::addSource()
    void addAction(PaymentActionImpl *action)
    {
        m_actions.push_back(action);
        m_annualPartitioning.addAction(m_actions.back());
    }
    virtual void addAction(PaymentActionType action, Database::ID objectId,
            Database::Date exDate = Database::Date(), unsigned int unitsCount = 0)
    {
        std::string foo;
        addAction(new PaymentActionImpl(Database::Money(), 0,
                    Database::Money(), foo, Database::DateTime(), exDate, 
                    action, unitsCount, Database::Money(), objectId));
    }
    void clearActions()
    {
        m_actions.clear();
    }
    virtual unsigned int getSourceCount() const 
    {
        return m_sources.size();
    }
    virtual const PaymentSource *getSource(unsigned int index) const
    {
        return (index >= m_sources.size()) ? NULL : m_sources.at(index);
    }
    virtual unsigned int getActionCount() const 
    {
        return m_actions.size();
    }
    virtual const PaymentAction *getAction(unsigned int index) const
    {
        return (index >= m_actions.size()) ? NULL : m_actions.at(index);
    }
    virtual AnnualPartitioning *getAnnualPartitioning()
    {
        return &m_annualPartitioning;
    }
    virtual unsigned int getPaymentCount() const
    {
        if (m_type == IT_DEPOSIT && !m_paid.size()) {
            ((InvoiceImpl *)this)->m_paid.push_back(
                PaymentImpl(getTotal(), getVatRate(), getTotalVAT()));
        }
        return m_paid.size();
    } // InvoiceImpl::getPaymentCount()
    virtual const Payment *getPayment(unsigned int index) const
    {
        return (index >= m_paid.size()) ? NULL : &m_paid.at(index);
    }
    virtual const Subject *getClient() const
    {
        return &m_client;
    }
    virtual const Subject *getSupplier() const 
    {
        return &m_supplier;
    }
    // this is not real type, but index of invoice prefix in
    // table ''invoice_prefix''
    virtual Database::ID getInvoicePrefixTypeId() const 
    {
        return m_invoicePrefixTypeId;
    }
    void setFile(Database::ID filePDF, Database::ID fileXML)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "setFile(Database::ID, Database::ID)");
        if (!m_filePDF) {
            m_filePDF = filePDF;
            m_fileXML = fileXML;

            m_storeFileFlag = true;
            try {
                storeFile();
            } catch (...)
            { }
        }
    } // InvoiceImpl::setFile()

    void storeFile()
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "storeFile()");
        if (m_storeFileFlag && getFilePDF()) {
            Database::Query updateFile;
            updateFile.buffer()
                << "UPDATE invoice SET file=" << getFilePDF();
            if (getFileXML()) {
                updateFile.buffer()
                    << ", filexml=" << getFileXML();
            };
            updateFile.buffer()
                << " WHERE id=" << getId();
            try {
                m_conn->exec(updateFile);
            } catch (...) {
                LOGGER(PACKAGE).error("Register::InvoiceImpl::storeFile(): error");
                throw;
            }
        }
    } // InvoiceImpl::storeFile()

    void doExport(Exporter *exp)
    {
        exp->doExport(this);
    }
    virtual void setId(Database::ID id)
    {
        id_ = id;
    }

    virtual void setZone(Database::ID zone) 
    {
        m_zone = zone;
    }

    virtual void setZoneName(std::string zoneName)
    {
        m_zoneName = zoneName;
    }

    virtual void setCrTime(Database::DateTime crTime) 
    {
        m_crTime = crTime;
    }
    virtual void setCrTime(std::string crTime) 
    {
        m_crTime = Database::DateTime(crTime);
    }
    virtual void setTaxDate(Database::Date taxDate) 
    {
        m_taxDate = taxDate;
    }
    virtual void setTaxDate(std::string taxDate) 
    {
        m_taxDate = Database::Date(taxDate);
    }
    virtual void setToDate(Database::Date toDate)
    {
        m_toDate = toDate;
    }
    virtual void setToDate(std::string toDate)
    {
        m_toDate = Database::Date(toDate);
    }
    virtual void setFromDate(Database::Date fromDate)
    {
        m_fromDate = fromDate;
    }
    virtual void setNumber(unsigned long long number) 
    {
        m_number = number;
    }
    virtual void setRegistrar(Database::ID registrar) 
    {
        m_registrar = registrar;
    }
    virtual void setRegistrarName(std::string registrar)
    {
        m_registrarName = registrar;
    }
    virtual void setCredit(Database::Money credit) 
    {
        m_credit = credit;
    }
    virtual void setPrice(Database::Money price) 
    {
        m_price = price;
    }
    virtual void setVATRate(int vatRate) 
    {
        m_vatRate = vatRate;
    }
    virtual void setTotal(Database::Money total) 
    {
        m_total = total;
    }
    virtual void setTotalVAT(Database::Money totalVat) 
    {
        m_totalVAT = totalVat;
    }
    virtual void setType(Type type) 
    {
        m_type = type;
    }
    virtual void setFileIdPDF(Database::ID id) 
    {
        m_filePDF = id;
    }
    virtual void setFileIdXML(Database::ID id) 
    {
        m_fileXML = id;
    }
    virtual void setManager(Manager *manager)
    {
        m_manager = manager;
    }
    virtual void setInvoicePrefixTypeId(Database::ID id) 
    {
        m_invoicePrefixTypeId = id;
    }
    virtual void setAccountPeriod(Database::DateInterval period)
    {
        m_accountPeriod = period;
    }

    /*! get operation (domain create or renew) price from database. */
    Database::Money getPrice(PaymentActionType operation, int period)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "getPrice(PaymentActionType, int)");
        Database::SelectQuery priceQuery;
        if (period > 0) {
            priceQuery.buffer() << "SELECT price * 100, period";
        } else {
            priceQuery.buffer() << "SELECT price * 100";
        }
        priceQuery.buffer()
            << " FROM price_list WHERE valid_from < 'now()' and "
            << "(valid_to is NULL or valid_to > 'now()')  and "
            << "operation=" << PaymentActionType2SqlType(operation)
            << " and zone=" << getZone();
        Database::Result priceResult = m_conn->exec(priceQuery);
        if (priceResult.size() == 0) {
            ERROR("Cannot get price from database");
            return Database::Money();
        }
        Database::Money money = *(*priceResult.begin()).begin();
        if (period > 0) {
            int pper = *(++(*priceResult.begin()).begin());
            return period * money / pper;
        } else {
            return money;
        }
    } // InvoiceImpl::getPrice()

    /*! return true is registrar is system */
    bool isRegistrarSystem(Database::ID registrar)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "isRegistrarSystem(Database::ID)");
        Database::SelectQuery regQuery;
        regQuery.buffer()
            << "SELECT system FROM registrar WHERE "
            << "id=" << registrar;
        Database::Result regResult = m_conn->exec(regQuery);
        if (regResult.size() == 0) {
            ERROR("Cannot find out if registrar is system");
            return false;
        }
        std::string system = *(*regResult.begin()).begin();
        if (system == "t") {
            return true;
        } else {
            return false;
        }
    } // InvoiceImpl::isRegistrarSystem()

    std::vector<Database::Money> countPayments(
            std::vector<Database::Money> credit,
            Database::Money price)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::countPayments("
                "std::vector<Database::Money>, std::vector<Database::Money>)");
        std::vector<Database::Money> payments;
        Database::Money remainder;
        for (int i = 0; i < (int)credit.size(); i++) {
            if (price > credit[i]) {
                price = price - credit[i];
                payments.push_back(credit[i]);
            } else {
                payments.push_back(price);
                return payments;
            }
        }
        if (price != Database::Money(0)) {
            ERROR("Registrar has insufficient credit");
            payments.clear();
        }
        return payments;
    } // InvoiceImpl::countPayments()

    /*! write new row into ``invoice_object_registry'' and return id
     * of newly added record
     */
    Database::ID newInvoiceObjectRegistry(Database::Transaction &transaction)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "newInvoiceObjectRegistry(Database::Transaction &)");

        Database::InsertQuery insertQuery("invoice_object_registry");
        insertQuery.add("objectid", getAction(0)->getObjectId());
        insertQuery.add("registrarid", getRegistrar());
        insertQuery.add("operation", PaymentActionType2SqlType(
                    getAction(0)->getAction()));
        insertQuery.add("zone", getZone());
        if (getAction(0)->getExDate() != Database::Date()) {
            insertQuery.add("period", getAction(0)->getUnitsCount());
            insertQuery.add("exdate", getAction(0)->getExDate());
        }
        try {
            transaction.exec(insertQuery);
        } catch (Database::Exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            throw;
        } catch (std::exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            throw;
        }
        Database::Sequence seq(*m_conn, "invoice_object_registry");
        Database::ID id = seq.getCurrent();
        return id;
    } // InvoiceImpl::newInvoiceObjectRegistry()

    /*! put new line into ``invoice_object_registry_price_map'', where
     * ``invObjRegId'' is id from table ``invoice_object_registry''
     */
    bool newInvoiceObjectRegistryPriceMap(
            Database::Transaction &transaction,
            Database::ID invObjRegId,
            Database::ID invoiceId,
            Database::Money price)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "newInvoiceObjectRegistryPriceMap("
                "Database::Transaction &, Database::ID)");
        Database::InsertQuery insertQuery("invoice_object_registry_price_map");
        insertQuery.add("id", invObjRegId);
        insertQuery.add("invoiceid", invoiceId);
        insertQuery.add("price", price);
        try {
            transaction.exec(insertQuery);
        } catch (Database::Exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            return false;
        } catch (std::exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            return false;
        }
        return true;
    } // InvoiceImpl::newInvoiceObjectRegistryPriceMap()

    bool updateInvoiceCredit(
            Database::Transaction &transaction,
            Database::ID invoiceId,
            Database::Money price)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "updateInvoiceCredit(Database::Transaction, "
                "Database::ID, Database::Money)");
        Database::Query query;
        query.buffer()
            << "UPDATE invoice SET credit=credit-" << price.format()
            << " WHERE id=" << invoiceId;
        try {
            transaction.exec(query);
        } catch (Database::Exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            return false;
        } catch (std::exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            return false;
        }
        return true;
    } // InvoiceImpl::updateInvoiceCredit()

    bool getCreditInvoicesId(
            std::vector<Database::ID> &vec_id,
            std::vector<Database::Money> &vec_money,
            int limit = 2)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "getCreditInvoices(int)");
        std::vector<Database::ID> ret;
        Database::Query selectQuery;
        selectQuery.buffer()
            << "SELECT id, credit * 100 FROM invoice WHERE "
            << "registrarid=" << getRegistrar()
            << " and zone=" << getZone()
            << " and credit > 0 order by id limit " << limit
            << " FOR UPDATE;";
        Database::Result res = m_conn->exec(selectQuery);
        if (res.size() == 0) {
            ERROR("cannot get two credit invoices");
            return false;
        }
        Database::Result::Iterator it = res.begin();
        for (; it != res.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();
            Database::ID    id      = *(col);
            Database::Money money   = *(++col);
            vec_id.push_back(id);
            vec_money.push_back(money);
        }
        return true;
    } // InvoiceImpl::getCreditInvoicesId()

    bool domainBilling(
            Database::Transaction &transaction,
            Database::ID zone, Database::ID registrar,
            Database::ID objectId,
            Database::Date exDate,
            int units_count)
    {
        LOGGER(PACKAGE).debug("[CALL] Register::Invoicing::InvoiceImpl::"
                "domainBilling(Database::Transaction &, Database::ID, "
                "Database::ID, Database::Date, int)");
        Database::Query query;
        query.buffer()
            << "select * from object_registry where id=" << objectId;
        Database::Result res = m_conn->exec(query);
        LOGGER(PACKAGE).debug(boost::format("result size: %1%")
                % res.size());
        setZone(zone);
        setRegistrar(registrar);
        addAction(PAT_CREATE_DOMAIN, objectId);
        if (!domainBilling(transaction)) {
            return false;
        }
        clearActions();
        addAction(PAT_RENEW_DOMAIN, objectId, exDate, units_count);
        if (!domainBilling(transaction)) {
            return false;
        }
        
        return true;
    } // InvoiceImpl::DomainBilling()

    bool domainBilling(
            Database::ID zone,
            Database::ID registrar,
            Database::ID objectId,
            Database::Date exDate,
            int units_count)
    {
        LOGGER(PACKAGE).debug("[CALL] Register::Invoicing::InvoiceImpl::"
                "domainBilling(Database::ID, Database::ID, "
                "Database::Date, int)");
        Database::Transaction transaction(*m_conn);
        if (domainBilling(transaction, zone, registrar, objectId,
                    exDate, units_count)) {
            transaction.commit();
            return true;
        }
        
        return false;
    } // InvoiceImpl::domainBilling()

    bool domainBilling(Database::Transaction &transaction)
    {
        LOGGER(PACKAGE).debug("[CALL] Register::Invoicing::InvoiceImpl::"
                "domainBilling(Database::Transaction)");
        if (isRegistrarSystem(getRegistrar())) {
            return true;
        }
        /* get price for desired operation */
        Database::Money price = getPrice(
                getAction(0)->getAction(), getAction(0)->getUnitsCount());
        if (price == Database::Money()) {
            ERROR("cannot retrieve price");
            return false;
        }
        std::vector<Database::ID> vec_invoiceId;
        std::vector<Database::Money> vec_money;
        if (!getCreditInvoicesId(vec_invoiceId, vec_money, 10)) {
            return false;
        }

        Database::ID invoiceObjectRegistryId = newInvoiceObjectRegistry(
                transaction);
        std::vector<Database::Money> vec_payments = countPayments(vec_money, price);
        if (vec_payments.empty()) {
            return false;
        }
        for (int i = 0; i < (int)vec_payments.size(); i++) {
            if (!newInvoiceObjectRegistryPriceMap(transaction,
                        invoiceObjectRegistryId, vec_invoiceId[i],
                        vec_payments[i])) {
                return false;
            }
            if (!updateInvoiceCredit(transaction, vec_invoiceId[i],
                        vec_payments[i])) {
                return false;
            }
        }
        return true;
    } // InvoiceImpl::domainBilling()

    /*! return proper 'from date' */
    Database::Date getFromDateFromDB()
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "getFromDateFromDB()");
        Database::Date fromDate;
        Database::SelectQuery fromDateQuery;
        fromDateQuery.buffer()
            << "SELECT date( todate + interval'1 day') "
            // << "SELECT todate "
            << "from invoice_generation  WHERE "
            << "zone=" << getZone()
            << " AND registrarid =" << getRegistrar()
            << " order by id desc limit 1;";
        Database::Result fromDateRes1 = m_conn->exec(fromDateQuery);
        if (fromDateRes1.size() == 0) {
            Database::SelectQuery fromDateQuery2;
            fromDateQuery2.buffer()
                << "SELECT fromdate from registrarinvoice WHERE"
                << " zone=" << getZone()
                << " and registrarid=" << getRegistrar() << ";";
            Database::Result fromDateRes2 = m_conn->exec(fromDateQuery2);
            if (fromDateRes2.size() == 0) {
                return fromDate;
            } else {
                fromDate = *(*fromDateRes2.begin()).begin();
            }
        } else {
            fromDate = *(*fromDateRes1.begin()).begin();
        }
        return fromDate;
    } // InvoiceImpl::getFromDateFromDB()

    /*! return number of record shich is abou tto invoicing */
    int getRecordsCount()
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "getRecordsCount()");
        int recordsCount;
        Database::SelectQuery recordCountQuery;
        recordCountQuery.buffer()
            << "SELECT count(id) from invoice_object_registry where"
            << " crdate < \'" << getCrTime() << "\'"
            << " AND  zone=" << getZone() 
            << " AND registrarid=" << getRegistrar()
            << " AND invoiceid IS NULL;";
        Database::Result recordCountRes = m_conn->exec(recordCountQuery);
        if (recordCountRes.size() == 0) {
            return -1;
        } else {
            recordsCount = *(*recordCountRes.begin()).begin();
        }
        return recordsCount;
    } // InvoiceImpl::getRecordsCount()

    /*! return price of all invoiced items */
    bool getRecordsPrice()
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "getRecordsPrice()");
        long recordsPrice;
        Database::SelectQuery recordsPriceQuery;
        recordsPriceQuery.buffer()
            << "SELECT CAST(sum(price)*100.0 AS INTEGER) FROM invoice_object_registry, "
            << "invoice_object_registry_price_map WHERE "
            << "invoice_object_registry_price_map.id=invoice_object_registry.id"
            << " AND  crdate < \'" <<  getCrTime() << "\'"
            << " AND zone=" << getZone()
            << " AND registrarid=" << getRegistrar()
            << " AND  invoice_object_registry.invoiceid is null;";
        Database::Result recordsPriceRes = m_conn->exec(recordsPriceQuery);
        if (recordsPriceRes.size() == 0) {
            return false;
        } else {
            recordsPrice = *(*recordsPriceRes.begin()).begin();
        }
        setPrice(recordsPrice);
        return true;
    } // InvoiceImpl::getRecordsPrice()
    
    /*! return valid vat value from ''price_vat'' table */
    int getSystemVAT()
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "getSystemVAT()");
        int vat;
        Database::SelectQuery vatQuery;
        vatQuery.buffer()
            << "select vat from price_vat "
            << "where valid_to > now() or valid_to is null;";
        Database::Result vatRes = m_conn->exec(vatQuery);
        if (vatRes.size() == 0) {
            return -1;
        } else {
            vat = *(*vatRes.begin()).begin();
        }
        return vat;
    } // InvoiceImpl::getSystemVAT()

    /*! check if table ``invoice_prefix'' contain any row with given year */
    bool isYearInInvoicePrefix(int year)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "isYearInInvoicePrefix(int)");
        Database::Query yearQuery;
        yearQuery.buffer()
            << "SELECT id FROM invoice_prefix WHERE year=" << year;
        Database::Result yearRes = m_conn->exec(yearQuery);
        if (yearRes.size() == 0) {
            return false;
        }
        return true;
    } // InvoiceImpl::isYearInInvoicePrefix()

    /*! create new rows in ``invoice_prefix'' from some
     * default values
     */
    void createNewYearInInvoicePrefix(
            int newYear, 
            Database::Transaction &transaction)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "createNewYearInInvoicePrefix(int, Database::Transaction)");
        unsigned long long year = (newYear - 2000) * 100000;
        Database::InsertQuery insert1("invoice_prefix");
        insert1.add("zone", 1);
        insert1.add("typ", 0);
        insert1.add("year", newYear);
        insert1.add("prefix", 110000001 + year);

        Database::InsertQuery insert2("invoice_prefix");
        insert2.add("zone", 1);
        insert2.add("typ", 1);
        insert2.add("year", newYear);
        insert2.add("prefix", 120000001 + year);

        Database::InsertQuery insert3("invoice_prefix");
        insert3.add("zone", 3);
        insert3.add("typ", 0);
        insert3.add("year", newYear);
        insert3.add("prefix", 130000001 + year);

        Database::InsertQuery insert4("invoice_prefix");
        insert4.add("zone", 3);
        insert4.add("typ", 1);
        insert4.add("year", newYear);
        insert4.add("prefix", 140000001 + year);
        try {
            transaction.exec(insert1);
            transaction.exec(insert2);
            transaction.exec(insert3);
            transaction.exec(insert4);
        } catch (Database::Exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            throw;
        } catch (std::exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            throw;
        }
    } // InvoiceImpl::createNewYearInInvoicePrefix()

    /*! create new rows in ``invoice_prefix'' table for new year
     * according oldYear
     */
    void createNewYearInInvoicePrefix(
            int newYear,
            int oldYear,
            Database::Transaction &transaction)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "createNewYearInInvoicePrefix(int, int, Database::Transaction)");
        Database::Query selectQuery;
        selectQuery.buffer()
            << "SELECT zone, typ, "
            << " CAST(substr(CAST(prefix AS varchar), 0, 3) AS integer) AS num"
            << " FROM invoice_prefix"
            << " WHERE year=" << oldYear
            << " ORDER BY num;";
        Database::Result selectRes = m_conn->exec(selectQuery);
        if (selectRes.size() == 0) {
            // oops, there is no year, from which i can copy
            createNewYearInInvoicePrefix(newYear, transaction);
            return;
        }
        Database::Result::Iterator it = selectRes.begin();

        // XXX works only for years 2000-2099
        unsigned long long year = (newYear - 2000) * 100000;

        for (; it != selectRes.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();
            Database::ID    zone    = *(col);
            int             type    = *(++col);
            int             num     = *(++col);

            unsigned long long prefix = num * 10000000 + 1 + year;

            Database::InsertQuery insertYear("invoice_prefix");
            insertYear.add("zone", zone);
            insertYear.add("typ", type);
            insertYear.add("year", newYear);
            insertYear.add("prefix", prefix);
            try {
                transaction.exec(insertYear);
            } catch (Database::Exception &ex) {
                ERROR(boost::format("%1%") % ex.what());
                throw;
            } catch (std::exception &ex) {
                ERROR(boost::format("%1%") % ex.what());
                throw;
            }
        }
    } // InvoiceImpl::createNewYearInInvoicePrefix()

    /*! get and update (i.e. add 1 to it) invoice number, save 
     * invoice prefix using ``setNumber()'' method, also get
     * invoice prefix type id (and save it using
     * ``setInvoicePredixTypeId()''
     */
    bool getInvoicePrefix(Database::Transaction &transaction)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "getInvoicePrefix(Database::Transaction)");
        long invoicePrefix;
        Database::ID id;
        Database::SelectQuery invoicePrefixQuery;
        invoicePrefixQuery.buffer()
            << "SELECT id, prefix FROM invoice_prefix WHERE"
            << " zone=" << getZone()
            << " AND typ=" << getType()
            << " AND year=\'" << getTaxDate().get().year() << "\';";
        Database::Result invoicePrefixRes = m_conn->exec(invoicePrefixQuery);
        if (invoicePrefixRes.size() == 0) {
            // try to create new prefix number(s) according rows from last year
            createNewYearInInvoicePrefix(
                    getTaxDate().get().year(), 
                    getTaxDate().get().year() - 1,
                    transaction);
            Database::Result invoicePrefixRes = m_conn->exec(invoicePrefixQuery);
            if (invoicePrefixRes.size() == 0) {
                return false;
            } else {
                Database::Row::Iterator col = (*invoicePrefixRes.begin()).begin();
                id = *col;
                invoicePrefix = *(++col);
            }
        } else {
            Database::Row::Iterator col = (*invoicePrefixRes.begin()).begin();
            id = *col;
            invoicePrefix = *(++col);
        }

        Database::Query updateInvoicePrefix;
        updateInvoicePrefix.buffer()
            << "update invoice_prefix "
            << "set prefix=" << invoicePrefix + 1
            << " where id=" << id << ";";
        try {
            transaction.exec(updateInvoicePrefix);
        } catch (Database::Exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            throw;
        } catch (std::exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            throw;
        }
        setNumber(invoicePrefix);
        setInvoicePrefixTypeId(id);
        return true;
    } // InvoiceImpl::getInvoicePrefix()

    /*! create new invoice */
    bool createNewInvoice(Database::Transaction &transaction)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::createNewInvoice("
                "Database::Transaction)");
        if (!getInvoicePrefix(transaction)) {
            return false;
        }
        Database::InsertQuery insertInvoice("invoice");
        insertInvoice.add("zone", getZone());
        insertInvoice.add("crdate", getCrTime());
        insertInvoice.add("taxdate", getTaxDate());
        insertInvoice.add("prefix", getNumber());
        insertInvoice.add("registrarid", getRegistrar());
        insertInvoice.add("prefix_type", getInvoicePrefixTypeId());
        insertInvoice.add("file", Database::ID());
        insertInvoice.add("filexml", Database::ID());
        insertInvoice.add("price", getPrice().format());
        insertInvoice.add("vat", getSystemVAT());
        // must insert NULL value
        insertInvoice.add("credit", Database::Value());
        try {
            transaction.exec(insertInvoice);
        } catch (Database::Exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            throw;
        } catch (std::exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            throw;
        }
        Database::Sequence seq(*m_conn, "invoice_id_seq");
        int invoiceId = seq.getCurrent();

        Database::InsertQuery insertGeneration("invoice_generation");
        insertGeneration.add("fromdate", getFromDate());
        insertGeneration.add("todate", getToDate());
        insertGeneration.add("registrarid", getRegistrar());
        insertGeneration.add("zone", getZone());
        insertGeneration.add("invoiceid", invoiceId);
        try {
            transaction.exec(insertGeneration);
        } catch (Database::Exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            throw;
        } catch (std::exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            throw;
        }
        id_ = invoiceId;

        return true;
    } // InvoiceImpl::createNewInvoice()

    /*! update record(s) in ''invoice_object_registry'' table - set id */
    void updateInvoiceObjectRegistry(Database::Transaction &transaction)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "updateInvoiceObjectRegistry(Database::Transaction)");
        Database::Query updateInvoiceObjectRegistryQuery;
        updateInvoiceObjectRegistryQuery.buffer()
            << "UPDATE invoice_object_registry"
            << " set invoiceid=" << id_
            << " WHERE crdate < \'" << getCrTime() << "\'"
            << " AND zone=" << getZone()
            << " and registrarid=" << getRegistrar()
            << " AND invoiceid IS NULL;";
        try {
            transaction.exec(updateInvoiceObjectRegistryQuery);
        } catch (Database::Exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            throw;
        } catch (std::exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            throw;
        }
    } // InvoiceImpl::updateInvoiceObjectRegistry()

    /*! update record(s) in ''registrarinvoice'' table - set lastdate */
    void updateRegistrarInvoice(Database::Transaction &transaction)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "updateRegistrarInvoice(Database::Transaction)");
        Database::Query updateRegistrarInvoiceQuery;
        updateRegistrarInvoiceQuery.buffer()
            << "UPDATE registrarinvoice SET"
            << " lastdate=\'" << getToDate() << "\'"
            << " WHERE zone=" << getZone()
            << " and registrarid=" << getRegistrar() << ";";
        try {
            transaction.exec(updateRegistrarInvoiceQuery);
        } catch (Database::Exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            throw;
        } catch (std::exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            throw;
        }
    } // InvoiceImpl::updateRegistrarInvoice()

    /*! return numbers of account invoices from which are object payed */
    std::vector<int> getAccountInvoicesNumbers(Database::Transaction &transaction)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "getAccountInvoicesNumbers(Database::Transaction)");
        std::vector<int> ret;
        Database::SelectQuery getInvoicesNumbersQuery;
        getInvoicesNumbersQuery.buffer()
            << "SELECT invoice_object_registry_price_map.invoiceid FROM"
            << " invoice_object_registry, invoice_object_registry_price_map"
            << " WHERE invoice_object_registry.id ="
            << " invoice_object_registry_price_map.id AND"
            << " invoice_object_registry.invoiceid=" << id_
            << " GROUP BY invoice_object_registry_price_map.invoiceid;" ;
        Database::Result res = m_conn->exec(getInvoicesNumbersQuery);
        Database::Result::Iterator it = res.begin();
        for (;it != res.end(); ++it) {
            int id = *(*it).begin();
            ret.push_back(id);
        }
        return ret;
    } // InvoiceImpl::getAccountInvoicesNumbers()

    long getInvoiceSumPrice(int accountInvoiceId)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "getInvoiceSumPrice(int)");
        long sumPrice;
        Database::SelectQuery sumPriceQuery;
        sumPriceQuery.buffer()
            << "SELECT CAST(SUM(invoice_object_registry_price_map.price) * 100.0 AS INTEGER)"
            << " FROM invoice_object_registry, invoice_object_registry_price_map"
            << " WHERE invoice_object_registry.id ="
            << " invoice_object_registry_price_map.id"
            << " AND invoice_object_registry.invoiceid=" << id_
            << " AND invoice_object_registry_price_map.invoiceid="
            << accountInvoiceId << ";";
        Database::Result sumPriceRes = m_conn->exec(sumPriceQuery);
        if (sumPriceRes.size() == 0) {
            return -1;
        } else {
            sumPrice = *(*sumPriceRes.begin()).begin();
        }
        return sumPrice;
    } // InvoiceImpl::getInvoiceSumPrice()

    long getInvoiceBalance(int accountInvoiceId, long invoiceSumPrice)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "getInvoiceBalance(int, long)");
        Database::SelectQuery totalQuery;
        totalQuery.buffer()
            << "SELECT CAST(total*100.0 AS INTEGER) FROM invoice "
            << "WHERE id="<< accountInvoiceId << ";";
        Database::Result totalResult = m_conn->exec(totalQuery);
        long total;
        if (totalResult.size() == 0) {
            return -1;
        } else {
            total = *(*totalResult.begin()).begin();
        }
        Database::SelectQuery sumQuery;
        sumQuery.buffer()
            << "SELECT CAST(SUM(credit)*100.0 AS INTEGER) "
            << "FROM invoice_credit_payment_map "
            << "WHERE ainvoiceid=" << accountInvoiceId << ";";
        Database::Result sumResult = m_conn->exec(sumQuery);
        long sum;
        if (sumResult.size() == 0) {
            return -1;
        } else {
            sum = *(*sumResult.begin()).begin();
        }
        long price = total - sum - invoiceSumPrice;
        return price;
    } // InvoiceImpl::getInvoiceBalance()

    void updateInvoiceCreditPaymentMap(std::vector<int> idNumbers,
            Database::Transaction &transaction)
    {
        LOGGER(PACKAGE).debug(
                "[CALL] Register::Invoicing::InvoiceImpl::"
                "updateInvoiceCreditPaymentMap("
                "std::vector<int>, Database::Transaction)");
        for (int i = 0; i < (int)idNumbers.size(); i++) {
            long invoiceSumPrice = getInvoiceSumPrice(idNumbers[i]);
            long invoiceBalance = getInvoiceBalance(idNumbers[i], invoiceSumPrice);
            Database::InsertQuery insertQuery("invoice_credit_payment_map");
            insertQuery.add("invoiceid", id_);
            insertQuery.add("ainvoiceid", idNumbers[i]);
            insertQuery.add("credit", Database::Money(invoiceSumPrice).format());
            insertQuery.add("balance", Database::Money(invoiceBalance).format());
            try {
                transaction.exec(insertQuery);
            } catch (Database::Exception &ex) {
                ERROR(boost::format("%1%") % ex.what());
                throw;
            } catch (std::exception &ex) {
                ERROR(boost::format("%1%") % ex.what());
                throw;
            }
        }
    } // InvoiceImpl::updateInvoiceCreditPaymentMap()

    bool testRegistrar()
    {
        if (getRegistrar() == Database::ID() && getRegistrarName().empty()) {
            ERROR("Registrar not set");
            return false;
        } else if (getRegistrar() == Database::ID()) {
            Database::SelectQuery regQuery;
            regQuery.buffer()
                << "select id from registrar where handle='"
                << getRegistrarName()
                << "'";
            Database::Result res = m_conn->exec(regQuery);
            if (res.size() == 0) {
                ERROR("registrar do not exists");
                return false;
            } else {
                setRegistrar(*(*res.begin()).begin());
            }
        } else if (getRegistrarName().empty()) {
            Database::SelectQuery query;
            query.buffer()
                << "select id from registrar where id="
                << getRegistrar();
            Database::Result res = m_conn->exec(query);
            if (res.size() == 0) {
                ERROR("registrar do not exists");
                return false;
            }
        } else {
            Database::SelectQuery query;
            query.buffer()
                << "select id from registrar where handle='"
                << getRegistrarName()
                << "' and id="
                << getRegistrar();
            Database::Result res = m_conn->exec(query);
            if (res.size() == 0) {
                ERROR("clash between registrar id and handle");
                return false;
            }
        }
        return true;
    } // InvoiceImpl::testRegistrar()

    bool testZone()
    {
        if (getZone() == 0 && getZoneName().empty()) {
            ERROR("Zone not set");
            return false;
        } else if (getZone() == 0) {
            Database::SelectQuery zoneQuery;
            zoneQuery.buffer()
                << "select zz.id from zone zz "
                << "join registrarinvoice rr on (rr.zone = zz.id) "
                << "where zz.fqdn='" << getZoneName() << "' "
                << "and rr.registrarid=" << getRegistrar() << " "
                << "and rr.fromdate <= date(now()) "
                << "limit 1";
            Database::Result res = m_conn->exec(zoneQuery);
            if (res.size() == 0) {
                ERROR("registrar do not belong to zone");
                return false;
            } else {
                setZone(*(*res.begin()).begin());
            }
        } else if (getZoneName().empty()) {
            Database::SelectQuery zoneQuery;
            zoneQuery.buffer()
                << "select zz.id from zone zz "
                << "join registrarinvoice rr on (rr.zone = zz.id) "
                << "where zz.id='" << getZone() << " "
                << "and rr.registrarid=" << getRegistrar() << " "
                << "and rr.fromdate <= date(now()) "
                << "limit 1";
            Database::Result res = m_conn->exec(zoneQuery);
            if (res.size() == 0) {
                ERROR("registrar do not belong to zone");
                return false;
            }
        } else {
            Database::SelectQuery zoneQuery;
            zoneQuery.buffer()
                << "select zz.id from zone zz "
                << "join registrarinvoice rr on (rr.zone = zz.id) "
                << "where zz.id=" << getZone() << " "
                << "and zz.fqdn='" << getZoneName() << "' "
                << "and rr.registrarid=" << getRegistrar() << " "
                << "and rr.fromdate < date(now()) limit 1";
            Database::Result res = m_conn->exec(zoneQuery);
            if (res.size() == 0) {
                ERROR("zone name does not correspond to zone id");
                return false;
            }
        }
        return true;
    } // InvoiceImpl::testZone()

    /*! create new account invoice */
    bool insertAccount()
    {
        TRACE("[CALL] Register::Invoicing::Invoice::insertAccount()");
        
        if (!testRegistrar()) {
            return false;
        }
        if (!testZone()) {
            return false;
        }

        if (getTaxDate() == Database::Date()) {
            ERROR("taxdate is not set");
            return false;
        }
        if (getToDate() == Database::Date()) {
            ERROR("todate is not set");
            return false;
        }
        if (getToDate() > Database::Date(Database::NOW)) {
            LOGGER(PACKAGE).info("To date is set to the future - using today");
            setToDate(Database::Date(Database::NOW));
        }

        Database::Date fromDate = getFromDateFromDB();
        if (fromDate == Database::Date()) {
            ERROR("cannot get fromDate");
            return false;
        }
        if (fromDate == (getToDate() + Database::Days(1))) {
            fromDate = getToDate();
        } else if (fromDate > getToDate()) {
            ERROR("fromDate cannot be bigger than toDate");
            return false;
        }
        setFromDate(fromDate);

        LOGGER(PACKAGE).debug(boost::format(
                    "taxdate: %1%, fromdate: %2%, todate: %3%")
                % getTaxDate() % fromDate % getToDate());
        setCrTime(Database::DateTime(Database::NOW));
        int recordsCount = getRecordsCount();
        if (recordsCount == -1) {
            ERROR("cannot get count of records to invoicing");
            return false;
        }
        if (recordsCount == 0) {
            ERROR("no records to invoice");
            return false;
        }
        if (!getRecordsPrice()) {
            ERROR("cannot get price of records to invoicing");
            return false;
        }
        LOGGER(PACKAGE).debug(boost::format(
                    "invoicing from %1% to %2%, tax date %3%, "
                    "create timestamp %4%, price %5%")
                % fromDate % getToDate() % getTaxDate() % getCrTime()
                % getPrice());

        Database::Transaction transaction(*m_conn);
        if (!createNewInvoice(transaction)) {
            ERROR("cannot create new invoice");
            return false;
        }
        if (recordsCount > 0) {
            updateInvoiceObjectRegistry(transaction);
        }
        updateRegistrarInvoice(transaction);
        std::vector<int> numbers = getAccountInvoicesNumbers(transaction);
        updateInvoiceCreditPaymentMap(numbers, transaction);
        transaction.commit();
        return true;
    } // InvoiceImpl::insertAccount()

    bool insertDeposit()
    {
        TRACE("[CALL] Register::Invoicing::InvoiceImpl::insertDeposit()");

        if (!testRegistrar()) {
            return false;
        }
        if (!testZone()) {
            return false;
        }

        if (getVatRate() == -1) {
            Database::SelectQuery vatQuery;
            vatQuery.buffer()
                << "select pv.vat from price_vat pv, registrar r "
                << "where r.vat=true and pv.valid_to isnull and "
                << "r.id=" << m_registrar << " limit 1";
            Database::Result res = m_conn->exec(vatQuery);
            if (res.size() != 0) {
                // get vat rate from database
                setVATRate(*(*res.begin()).begin());
            } else {
                // XXX default czech value
                setVATRate(19);
            }
        }
        Database::Transaction transaction(*m_conn);
        if (getNumber() == 0) {
            if (!getInvoicePrefix(transaction)) {
                ERROR("Cannot obtain invoice number");
                return false;
            }
        }

        setTotalVAT(m_manager->countVat(getPrice(), getVatRate(), false));
        setCredit(getPrice() - getTotalVAT());
        setTotal(getCredit());
        
        Database::InsertQuery insert("invoice");
        insert.add("zone", getZone());
        if (getCrTime() != Database::DateTime()) {
            insert.add("crdate", getCrTime());
        }
        insert.add("taxdate", (getTaxDate() != Database::Date()) ?
                getTaxDate() : Database::Date(day_clock::local_day()));
        insert.add("prefix", getNumber());
        insert.add("registrarid", getRegistrar());
        insert.add("credit", getCredit().format());
        insert.add("price", getPrice().format());
        insert.add("vat", getVatRate());
        insert.add("total", getTotal().format());
        insert.add("totalvat", getTotalVAT().format());
        insert.add("prefix_type", getInvoicePrefixTypeId());
        insert.add("file", (getFilePDF()) ? getFilePDF() : Database::ID());
        insert.add("filexml", (getFileXML()) ? getFileXML() : Database::ID());
        try {
            assert(m_conn);
            transaction.exec(insert);
            Database::Sequence seq(*m_conn, "invoice_id_seq");
            id_ = seq.getCurrent();
            LOGGER(PACKAGE).info(boost::format(
                        "invoice item id='%1%' created successfully")
                    % id_);
        } catch (Database::Exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            return false;
        } catch (std::exception &ex) {
            ERROR(boost::format("%1%") % ex.what());
            return false;
        }
        transaction.commit();
        return true;
    } // InvoiceImpl::insertDeposit()
    bool updateAccount()
    {
        TRACE("[CALL] Register::Invoicing::Invoice::updateAccount()");
        LOGGER(PACKAGE).debug("not implemented");
        return false;
    }
    bool updateDeposit()
    {
        TRACE("[CALL] Register::Invoicing::Invoice::updateDeposit()");
        LOGGER(PACKAGE).debug("not implemented");
        return false;
    }
    bool update()
    {
        TRACE("[CALL] Register::Invoicing::Invoice::update()");
        Database::Query updateDeposit;
        updateDeposit.buffer()
            << "UPDATE invoice SET "
            << "zone=" << getZone()
            << ", crdate='" << getCrTime() << "'"
            << ", taxdate='" << getTaxDate() << "'"
            << ", prefix=" << getNumber()
            << ", registrarid=" << getRegistrar()
            << ", credit=" << getCredit().format()
            << ", price=" << getPrice().format()
            << ", vat=" << getVatRate()
            << ", total=" << getTotal().format()
            << ", totalvat=" << getTotalVAT().format()
            << ", prefix_type=" << getInvoicePrefixTypeId()
            << ", file=" << transformId(getFilePDF())
            << ", filexml=" << transformId(getFileXML())
            << " WHERE id=" << id_;
        std::cout << updateDeposit << std::endl;
        try {
            Database::Transaction transaction(*m_conn);
            transaction.exec(updateDeposit);
            transaction.commit();
            LOGGER(PACKAGE).info(boost::format(
                        "invoice id='%1%' updated successfully")
                    % id_);
        } catch (Database::Exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            return false;
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            return false;
        }
        return true;
    } // InvoiceImpl::update()
    virtual bool save() 
    {
        TRACE("[CALL] Register::Invoicing::Invoice::save()");
        if (id_) {
            update();
            // if (getType() == IT_ACCOUNT) {
                // return updateAccount();
            // } else if (getType() == IT_DEPOSIT) {
                // return updateDeposit();
            // }
        } else {
            if (getType() == IT_ACCOUNT) {
                return insertAccount();
            } else if (getType() == IT_DEPOSIT) {
                return insertDeposit();
            }
        }
        return true;
    }
}; // class InvoiceImpl

SubjectImpl InvoiceImpl::m_supplier(0, "REG-CZNIC", "CZ.NIC, z.s.p.o.",
        "CZ.NIC, zájmové sdružení právnických osob", "Americká 23", 
        "Praha 2", "120 00", "CZ", "67985726", "CZ67985726", 
        "SpZ: odb. občanskopr. agend Magist. hl. m. Prahy, č. ZS/30/3/98", 
        "CZ.NIC, z.s.p.o., Americká 23, 120 00 Praha 2", "www.nic.cz", 
        "podpora@nic.cz", "+420 222 745 111", "+420 222 745 112", 1);

// forward declaration
class ExporterXML:
    public Exporter {
private:
    std::ostream    &m_out;
    bool            m_xmlDecl;
public:
    ExporterXML(std::ostream &out, bool xmlDecl):
        m_out(out), m_xmlDecl(xmlDecl)
    { }

    std::ostream &doExport(const Subject *subj)
    {
        m_out
            << TAG(id,subj->getId()) 
            << TAG(name,subj->getName()) 
            << TAG(fullname,subj->getFullname()) 
            << TAGSTART(address) 
            << TAG(street,subj->getStreet()) 
            << TAG(city,subj->getCity()) 
            << TAG(zip,subj->getZip()) 
            << TAG(country,subj->getCountry()) 
            << TAGEND(address) 
            << TAG(ico,subj->getICO()) 
            << TAG(vat_number,subj->getVatNumber()) 
            << TAG(registration,subj->getRegistration()) 
            << TAG(reclamation,subj->getReclamation()) 
            << TAG(url,subj->getURL()) 
            << TAG(email,subj->getEmail()) 
            << TAG(phone,subj->getPhone()) 
            << TAG(fax,subj->getFax()) 
            << TAG(vat_not_apply,(subj->getVatApply() ? 0 : 1));
        return m_out;
    }

    virtual void doExport(Invoice *inv)
    {
        m_out.imbue(std::locale(
                    std::locale(
                        m_out.getloc(),
                        new time_facet("%Y-%m-%d %T")
                        ),
                    new date_facet("%Y-%m-%d")
                    ));
        // generate invoice xml
        if (m_xmlDecl)
            m_out << "<?xml version='1.0' encoding='utf-8'?>";
        m_out << TAGSTART(invoice)
            << TAGSTART(client);
        doExport(inv->getClient());
        m_out << TAGEND(client)
            << TAGSTART(supplier);
        doExport(inv->getSupplier());
        m_out << TAGEND(supplier)
            << TAGSTART(payment)
            << TAG(invoice_number,inv->getNumber())
            << TAG(invoice_date,inv->getCrTime().get().date());
        if (inv->getType() == IT_DEPOSIT)
            m_out << TAG(advance_payment_date,inv->getTaxDate());
        else {
            m_out << TAG(tax_point,inv->getTaxDate())
                << TAG(period_from,inv->getAccountPeriod().begin())
                << TAG(period_to,inv->getAccountPeriod().end());
        }
        m_out << TAG(vs,inv->getVarSymbol())
            << TAGEND(payment)
            << TAGSTART(delivery)
            << TAGSTART(vat_rates);
        for (unsigned j=0; j<inv->getPaymentCount(); j++) {
            const Payment *p = inv->getPayment(j);
            m_out << TAGSTART(entry)
                << TAG(vatperc,p->getVatRate())
                << TAG(basetax,p->getPrice().format())
                << TAG(vat,p->getVat().format())
                << TAG(total,p->getPriceWithVat().format())
                << TAGSTART(years);
            for (
                    inv->getAnnualPartitioning()->resetIterator(p->getVatRate());
                    !inv->getAnnualPartitioning()->end();
                    inv->getAnnualPartitioning()->next()
                ) {
                AnnualPartitioning *ap = inv->getAnnualPartitioning();
                m_out << TAGSTART(entry)
                    << TAG(year,ap->getYear())
                    << TAG(price,ap->getPrice().format())
                    << TAG(vat,ap->getVat().format())
                    << TAG(total,ap->getPriceWithVat().format())
                    << TAGEND(entry);
            }
            m_out << TAGEND(years)
                << TAGEND(entry);
        }
        m_out << TAGEND(vat_rates)
            << TAGSTART(sumarize)
            << TAG(total,inv->getPrice().format());
        if (inv->getType() != IT_DEPOSIT) {
            m_out << TAG(paid, OUTMONEY(-inv->getPrice()));
        } else {
            m_out << TAG(paid, OUTMONEY(0));
        }
        m_out
            << TAG(to_be_paid,OUTMONEY(0))
            << TAGEND(sumarize)
            << TAGEND(delivery);
        if (inv->getSourceCount()) {
            m_out << TAGSTART(advance_payment)
                << TAGSTART(applied_invoices);
            for (unsigned k=0; k<inv->getSourceCount(); k++) {
                const PaymentSource *ps = inv->getSource(k);
                m_out << TAGSTART(consumed)
                    << TAG(number,ps->getNumber())
                    << TAG(price,ps->getPrice().format())
                    << TAG(balance,ps->getCredit().format())
                    << TAG(vat,ps->getVat().format())
                    << TAG(vat_rate,ps->getVatRate())
                    << TAG(pricevat,ps->getPriceWithVat().format())
                    << TAG(total,ps->getTotalPrice().format())
                    << TAG(total_vat,ps->getTotalVat().format())
                    << TAG(total_with_vat,ps->getTotalPriceWithVat().format())
                    << TAG(crtime,ps->getCrTime())
                    << TAGEND(consumed);
            }
            m_out << TAGEND(applied_invoices)
                << TAGEND(advance_payment);
        }
        if (inv->getActionCount()) {
            m_out << TAGSTART(appendix)
                << TAGSTART(items);
            for (unsigned k=0; k<inv->getActionCount(); k++) {
                const PaymentAction *pa = inv->getAction(k);
                m_out << TAGSTART(item)
                    << TAG(subject,pa->getObjectName())
                    << TAG(code,
                            (pa->getAction() == PAT_CREATE_DOMAIN ?
                             "RREG" : "RUDR"))
                    << TAG(timestamp,pa->getActionTime());
                if (!pa->getExDate().is_special())
                    m_out << TAG(expiration,pa->getExDate());
                m_out << TAG(count,pa->getUnitsCount()/12) // in years
                    << TAG(price,pa->getPricePerUnit().format())
                    << TAG(total,pa->getPrice().format())
                    << TAG(vat_rate,pa->getVatRate())
                    << TAGEND(item);
            }
            m_out << TAGEND(items)
                << TAGSTART(sumarize_items)
                << TAG(total,inv->getPrice().format())
                << TAGEND(sumarize_items)
                << TAGEND(appendix);
      }
      m_out << TAGEND(invoice);
    } // void doExport(Invoice *)
}; // class ExporterXML


class ListImpl:
    public Register::CommonListImpl,
    virtual public List {
private:
    Manager     *m_manager;
    bool        m_partialLoad;
    Database::Manager *m_dbMan;
public:

    ListImpl(Database::Manager *dbMan, Database::Connection *conn,
            Manager *manager):
        CommonListImpl(conn),
        m_manager(manager),
        m_partialLoad(false),
        m_dbMan(dbMan)
    { }

    virtual void setPartialLoad(bool partialLoad)
    {
        m_partialLoad = partialLoad;
    }
    
    virtual Invoice *get(unsigned int index) const
    {
        try {
            Invoice *inv = 
                dynamic_cast<Invoice *>(data_.at(index));
            if (inv) {
                return inv;
            } else {
                throw std::exception();
            }
        } catch (...) {
            throw std::exception();
        }
    }

    virtual void reload(Database::Filters::Union &filter, Database::Manager *dbman = NULL)
    {
        TRACE("[CALL] Register::Invoicing::ListImpl::reload(Database::Filters::Union &)");
        clear();
        filter.clearQueries();

        bool at_least_one = false;
        Database::SelectQuery id_query;
        Database::Filters::Compound::iterator iit = filter.begin();
        for (; iit != filter.end(); ++ iit) {
            Database::Filters::Invoice *invFilter =
                dynamic_cast<Database::Filters::Invoice *>(*iit);
            if (!invFilter) {
                continue;
            }
            Database::SelectQuery *tmp = new Database::SelectQuery();
            tmp->addSelect(new Database::Column(
                        "id", invFilter->joinInvoiceTable(), "DISTINCT"));
            filter.addQuery(tmp);
            at_least_one = true;
        }
        if (!at_least_one) {
            LOGGER(PACKAGE).error("wrong filter passed for reload!");
            return;
        }
        id_query.limit(load_limit_);
        filter.serialize(id_query);
        Database::InsertQuery tmp_table_query =
            Database::InsertQuery(getTempTableName(), id_query);
        LOGGER(PACKAGE).debug(boost::format(
                    "temporary table '%1%' generated sql = %2%")
                % getTempTableName() % tmp_table_query.str());

        Database::SelectQuery object_info_query;
        object_info_query.select()
            << "t_1.id, t_1.zone, t_2.fqdn, t_1.crdate, t_1.taxdate, "
            << "t_5.fromdate, t_5.todate, t_4.typ, t_1.prefix, "
            << "t_1.prefix_type, "
            << "t_1.registrarid, t_1.credit * 100, t_1.price * 100, "
            << "t_1.vat, t_1.total * 100, t_1.totalvat * 100, "
            << "t_1.file, t_1.fileXML, t_3.organization, t_3.street1, "
            << "t_3.city, t_3.postalcode, "
            << "TRIM(t_3.ico), TRIM(t_3.dic), TRIM(t_3.varsymb), "
            << "t_3.handle, t_3.vat, t_3.id, t_3.country, "
            << "t_6.name as file_name, t_7.name as filexml_name";
        object_info_query.from()
            << "tmp_invoice_filter_result tmp "
            << "JOIN invoice t_1 ON (tmp.id = t_1.id) "
            << "JOIN zone t_2 ON (t_1.zone = t_2.id) "
            << "JOIN registrar t_3 ON (t_1.registrarid = t_3.id) "
            << "JOIN invoice_prefix t_4 ON (t_4.id = t_1.prefix_type) "
            << "LEFT JOIN invoice_generation t_5 ON (t_1.id = t_5.invoiceid) "
            << "LEFT JOIN files t_6 ON (t_1.file = t_6.id) "
            << "LEFT JOIN files t_7 ON (t_1.filexml = t_7.id)";
        object_info_query.order_by()
            << "tmp.id";
        try {
            fillTempTable(tmp_table_query);
            Database::Result r_info = conn_->exec(object_info_query);
            for (Database::Result::Iterator it = r_info.begin();
                    it != r_info.end(); ++it) {
                Database::Row::Iterator col = (*it).begin();

                Database::ID       id             = *col;
                Database::ID       zone           = *(++col);
                std::string        fqdn           = *(++col);
                Database::DateTime create_time    = *(++col);
                Database::Date     tax_date       = *(++col);
                Database::Date     from_date      = *(++col);
                Database::Date     to_date        = *(++col);
                Type               type           =
                    (int)*(++col) == 0 ? IT_DEPOSIT : IT_ACCOUNT;
                unsigned long long number         = *(++col);
                int                prefix_type    = *(++col);
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

                Database::DateInterval account_period(from_date, to_date);
                SubjectImpl client(c_id, c_handle, c_organization, "", c_street1,
                        c_city, c_postal_code, c_country, c_ico, c_dic,
                        "", "", "", "", "", "", c_vat);

                assert(m_dbMan);
                InvoiceImpl *invoice = new InvoiceImpl(id,
                        zone, fqdn, create_time, tax_date, account_period,
                        type, number, registrar_id, credit, price, vat_rate,
                        total, total_vat, filePDF, fileXML, c_var_symb,
                        client, filepdf_name, filexml_name, m_dbMan, conn_,
                        m_manager);
                invoice->setInvoicePrefixTypeId(prefix_type);
                data_.push_back(invoice);
                LOGGER(PACKAGE).debug(boost::format(
                            "list of invoices size: %1%")
                        % data_.size());
            }
            if (data_.empty()) {
                return;
            }
            /* load details to each invoice */
            resetIDSequence();
            Database::SelectQuery source_query;
            source_query.select()
                << "tmp.id, ipm.credit * 100, sri.vat, sri.prefix, "
                << "ipm.balance * 100, sri.id, sri.total * 100, "
                << "sri.totalvat * 100, sri.crdate";
            source_query.from()
                << "tmp_invoice_filter_result tmp "
                << "JOIN invoice_credit_payment_map ipm ON (tmp.id = ipm.invoiceid) "
                << "JOIN invoice sri ON (ipm.ainvoiceid = sri.id) ";
            source_query.order_by()
                << "tmp.id";
            resetIDSequence();
            Database::Result r_sources = conn_->exec(source_query);
            for (Database::Result::Iterator it = r_sources.begin();
                    it != r_sources.end(); ++it) {
                Database::Row::Iterator col = (*it).begin();
                Database::ID invoiceId = *col;
                InvoiceImpl *invoicePtr =
                    dynamic_cast<InvoiceImpl *>(findIDSequence(invoiceId));
                if (invoicePtr) {
                    Database::Money     price   = *(++col);
                    unsigned int        vatRate = *(++col);
                    unsigned long long  number  = *(++col);
                    Database::Money     credit  = *(++col);
                    Database::ID        id      = *(++col);
                    Database::Money     totalPrice  = *(++col);
                    Database::Money     totalVat    = *(++col);
                    Database::DateTime  crTime  = *(++col);
                    Database::Money     vat = 
                        m_manager->countVat(price, vatRate, true);
                    PaymentSourceImpl *newSource = new PaymentSourceImpl(
                            price, vatRate, vat, number, credit, id, totalPrice,
                            totalVat, crTime);
                    invoicePtr->addSource(newSource);
                }
            }
            /* append list of actions to all selected invoices
             * it handle situation when action come from source advance invoices
             * with different vat rates by grouping
             * this is ignored on partial load
             */
            if (!m_partialLoad) {
                Database::SelectQuery actionQuery;
                actionQuery.select()
                    << "tmp.id, SUM(ipm.price) * 100, i.vat, o.name, "
                    << "ior.crdate, ior.exdate, ior.operation, ior.period, "
                    << "CASE "
                    << "  WHEN ior.period = 0 THEN 0 "
                    << "  ELSE 100 * SUM(ipm.price) * 12 / ior.period END, "
                    << "o.id";
                actionQuery.from() << "tmp_invoice_filter_result tmp "
                    << "JOIN invoice_object_registry ior ON (tmp.id = ior.invoiceid) "
                    << "JOIN object_registry o ON (ior.objectid = o.id) "
                    << "JOIN invoice_object_registry_price_map ipm ON (ior.id = ipm.id) "
                    << "JOIN invoice i ON (ipm.invoiceid = i.id) ";
                actionQuery.group_by() 
                    << "tmp.id, o.name, ior.crdate, ior.exdate, "
                    << "ior.operation, ior.period, o.id, i.vat";
                actionQuery.order_by() 
                    << "tmp.id";
                
                resetIDSequence();
                Database::Result r_actions = conn_->exec(actionQuery);
                Database::Result::Iterator it = r_actions.begin();
                for (; it != r_actions.end(); ++it) {
                    Database::Row::Iterator col = (*it).begin();
                    Database::ID invoiceId = *col;
                    InvoiceImpl *invoicePtr =
                        dynamic_cast<InvoiceImpl *>(findIDSequence(invoiceId));
                    if (invoicePtr) {
                        Database::Money     price       = *(++col);
                        unsigned int        vatRate     = *(++col);
                        std::string         objectName  = *(++col);
                        Database::DateTime  actionTime  = *(++col);
                        Database::Date      exDate      = *(++col);
                        PaymentActionType   type        = 
                            (int)*(++col) == 1 ? PAT_CREATE_DOMAIN : PAT_RENEW_DOMAIN;
                        unsigned int        units       = *(++col);
                        Database::Money     pricePerUnit= *(++col);
                        Database::ID        id          = *(++col);
                        Database::Money     vat         = m_manager->countVat(price, vatRate, true);
                        PaymentActionImpl *action
                            = new PaymentActionImpl(price, vatRate, vat,
                                    objectName, actionTime, exDate, type,
                                    units, pricePerUnit, id);
                        invoicePtr->addAction(action);
                    }
                }
            } //partialLoad
            CommonListImpl::reload();
        } catch (Database::Exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            clear();
        } catch (std::exception &ex) {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            clear();
        }
    } // ListImpl::reload(Database::Filters::Union &)

    virtual void sort(MemberType member, bool asc)
    {
        // TODO!!!!!!sakra
        LOGGER(PACKAGE).debug("Register::Invoicing::ListImpl::sort not implemented");
    }

    void doExport(Exporter *exp)
    {
        Iterator it = data_.begin();
        for (; it != data_.end(); ++it) {
            InvoiceImpl *invoice = dynamic_cast<InvoiceImpl*>(*it);
            if (invoice) {
                invoice->doExport(exp);
            }
        }
    }

    virtual void exportXML(std::ostream &out)
    {
        out << "<?xml version='1.0' encoding='utf-8'?>";
        ExporterXML xml(out,false);
        if (getCount() != 1) {
            out << TAGSTART(list);
        }
        doExport(&xml);
        if (getCount() != 1) {
            out << TAGEND(list);
        }
    }

    virtual const char *getTempTableName() const
    {
        return "tmp_invoice_filter_result";
    }

    virtual void makeQuery(bool, bool, std::stringstream &) const
    { }

    virtual void reload()
    { }

}; // class ListImpl

class Mails {
private:
    struct Item {
        std::string     m_registrarEmail;
        Database::Date  m_from;
        Database::Date  m_to;
        Database::ID    m_filePDF;
        Database::ID    m_fileXML;
        Database::ID    m_generation;
        Database::ID    m_invoice;
        Database::ID    m_mail;

        std::string getTemplateName()
        {
            if (!m_generation) {
                return "invoice_deposit";
            }
            if (!m_invoice) {
                return "invoice_noaudit";
            }
            return "invoice_audit";
        }
        Item(const std::string &registrarEmail, Database::Date from,
                Database::Date to, Database::ID filePDF, Database::ID fileXML,
                Database::ID generation, Database::ID invoice,
                Database::ID mail):
            m_registrarEmail(registrarEmail), m_from(from), m_to(to),
            m_filePDF(filePDF), m_fileXML(fileXML), m_generation(generation),
            m_invoice(invoice), m_mail(mail)
        { }
    }; // struct Item;

    typedef std::vector<Item> MailItems;
    MailItems               m_items;
    Mailer::Manager         *m_mailMan;
    Database::Manager       *m_dbMan;
    Database::Connection    *m_conn;

    void store(unsigned int index)
    {
        Database::InsertQuery insertMail("invoice_mails");
        insertMail.add("invoiceid", m_items[index].m_invoice);
        insertMail.add("genid", m_items[index].m_generation);
        insertMail.add("mailid", m_items[index].m_mail);
        m_conn->exec(insertMail);
    }
public:
    Mails(Mailer::Manager *mailMan, Database::Manager *dbMan):
        m_mailMan(mailMan), m_dbMan(dbMan),
        m_conn(dbMan->getConnection())
    { }
    Mails(Mailer::Manager *mailMan, Database::Connection *conn):
        m_mailMan(mailMan), m_dbMan(NULL),
        m_conn(conn)
    { }
    void send()
    {
        for (unsigned int i = 0; i < m_items.size(); i++) {
            Item *item = &m_items[i];
            Mailer::Parameters params;
            std::stringstream dateBuffer;
            dateBuffer.imbue(std::locale(dateBuffer.getloc(),
                        new date_facet("%d.%m.%Y")));
            dateBuffer << item->m_from;
            params["fromdate"] = dateBuffer.str();
            dateBuffer.str("");
            dateBuffer << item->m_to;
            params["todate"] = dateBuffer.str();
            Mailer::Handles handles;
            // TODO: include domain or registrar handles??
            Mailer::Attachments attach;
            if (item->m_filePDF) {
                attach.push_back(item->m_filePDF);
            }
            if (item->m_fileXML) {
                attach.push_back(item->m_fileXML);
            }
            item->m_mail = m_mailMan->sendEmail("", item->m_registrarEmail, 
                    "", item->getTemplateName(), params, handles, attach);
            if (!item->m_mail) {
                LOGGER(PACKAGE).error("Register::Invoicing::Mails::send() error");
            }
            store(i);
        }
    } // Mails::send()
    void load()
    {
        Database::Query loadMailsQuery;
        loadMailsQuery.buffer() 
            << "SELECT r.email, g.fromdate, g.todate, "
            << "i.file, i.fileXML, g.id, i.id "
            << "FROM registrar r, invoice i "
            << "LEFT JOIN invoice_generation g ON (g.invoiceid=i.id) "
            << "LEFT JOIN invoice_mails im ON (im.invoiceid=i.id)"
            << "WHERE i.registrarid=r.id "
            << "AND im.mailid ISNULL "
            << "UNION "
            << "SELECT r.email, g.fromdate, g.todate, NULL, NULL, g.id, NULL "
            << "FROM registrar r, invoice_generation g "
            << "LEFT JOIN invoice_mails im ON (im.genid=g.id) "
            << "WHERE g.registrarid=r.id AND g.invoiceid ISNULL "
            << "AND im.mailid ISNULL "
            << "AND NOT(r.email ISNULL OR TRIM(r.email)='')";
        Database::Result loadMailsResult = m_conn->exec(loadMailsQuery);
        Database::Result::Iterator it = loadMailsResult.begin();
        for (; it != loadMailsResult.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();

            std::string         email       = *(col);
            Database::Date      from        = *(++col);
            Database::Date      to          = *(++col);
            Database::ID        filePDF     = *(++col);
            Database::ID        fileXML     = *(++col);
            Database::ID        generation  = *(++col);
            Database::ID        invoice     = *(++col);
            Database::ID        mail        = *(++col);
            m_items.push_back(Item(email, from, to, filePDF, fileXML,
                        generation, invoice, mail));
        }
    } // Mails::load()
}; // class Mails

class ExporterArchiver:
    public Exporter {
private:
    Document::Manager *m_docMan;
    std::string makeFileName(Invoice *inv, std::string suffix)
    {
        std::stringstream filename;
        filename << inv->getNumber() << suffix;
        return filename.str();
    }
public:
    ExporterArchiver(Document::Manager *docMan):
        m_docMan(docMan)
    { }
    virtual void doExport(Invoice *inv)
    {
        try {
            std::auto_ptr<Document::Generator>
                gPDF(m_docMan->createSavingGenerator(
                            inv->getType() == IT_DEPOSIT ?
                                Document::GT_ADVANCE_INVOICE_PDF :
                                Document::GT_INVOICE_PDF,
                            makeFileName(inv, ".pdf"),
                            INVOICE_PDF_FILE_TYPE,
                            inv->getClient()->getCountry() == "CZ" ?
                                "cs" : "en")
                    );
            ExporterXML(gPDF->getInput(), true).doExport(inv);
            Database::ID filePDF = gPDF->closeInput();
            std::auto_ptr<Document::Generator>
                gXML(m_docMan->createSavingGenerator(
                            Document::GT_INVOICE_OUT_XML,
                            makeFileName(inv, ".xml"),
                            INVOICE_XML_FILE_TYPE,
                            inv->getClient()->getVatApply() ? "cs" : "en")
                    );
            ExporterXML(gXML->getInput(), true).doExport(inv);
            Database::ID fileXML = gXML->closeInput();
            InvoiceImpl *invoice = dynamic_cast<InvoiceImpl *>(inv);
            invoice->setFile(filePDF, fileXML);
        } catch (...) {
            LOGGER(PACKAGE).error(
                    "Register::Invoicing::ExporterArchiver::doExport("
                    "Invoice *): exception catched");
        }
    } // ExporterArchiver::doExport()

}; // class ExporterArchiver

class VAT {
private:
    unsigned int        m_vatRate;
    unsigned int        m_koef;
    Database::DateTime  m_validity;
public:
    VAT(unsigned int vatRate, unsigned int koef, Database::DateTime validity):
        m_vatRate(vatRate), m_koef(koef), m_validity(validity)
    { }
    bool operator==(unsigned rate) const
    {
        return m_vatRate == rate;
    }
    unsigned int getVatRate() const
    {
        return m_vatRate;
    }
    unsigned int getKoef() const
    {
        return m_koef;
    }
    Database::DateTime getValidity() const
    {
        return m_validity;
    }
}; // class VAT;

class ManagerImpl:
    virtual public Manager {
private:
    Database::Connection    *m_conn;
    Database::Manager       *m_dbMan;
    Document::Manager       *m_docMan;
    Mailer::Manager         *m_mailMan;
    std::vector<VAT>        m_vatList;
    void initVatList();
    const VAT *getVat(unsigned int rate) const;
public:
    ManagerImpl(Database::Manager *dbMan, Document::Manager *docMan = NULL,
            Mailer::Manager *mailMan = NULL):
        m_conn(dbMan->getConnection()),
        m_dbMan(dbMan),
        m_docMan(docMan),
        m_mailMan(mailMan)
    { 
        initVatList();
    }
    
    virtual ~ManagerImpl()
    {
        boost::checked_delete<Database::Connection>(m_conn);
    }

    List *createList() const
    {
        return new ListImpl(m_dbMan, m_conn, (Manager *)this);
    }

    Database::Money countVat(Database::Money price, unsigned int vatRate,
            bool base);
             
    virtual Invoice *createInvoice(Type type)
    {
        InvoiceImpl *invoice = new InvoiceImpl(m_dbMan, m_conn, (Manager *)this);
        invoice->setType(type);
        return invoice;
    }

    virtual Invoice *createAccountInvoice()
    {
        return createInvoice(IT_ACCOUNT);
    }

    virtual Invoice *createDepositInvoice() 
    {
        return createInvoice(IT_DEPOSIT);
    }

    virtual Database::Money getCreditByZone(
            const std::string &registrarHandle, Database::ID zoneId) const;
    virtual void archiveInvoices(bool send) const;
}; // class ManagerImpl

Manager *
Manager::create(Database::Manager *dbMan, Document::Manager *docMan,
        Mailer::Manager *mailMan)
{
    TRACE("[CALL] Register::Invoicing::Manager::create(Database::Manager *)");
    return new ManagerImpl(dbMan, docMan, mailMan);
}

Database::Money
ManagerImpl::getCreditByZone(
        const std::string &registrarHandle, Database::ID zoneId) const
{
    Database::SelectQuery getCreditQuery;
    getCreditQuery.buffer()
        << "SELECT SUM(credit) * 100 FROM invoice i JOIN registrar r "
        << "ON (i.registrar=r.id) "
        << "WHERE i.zone=" << zoneId
        << " AND r.handle='" << registrarHandle << "';";
    Database::Result getCreditRes = m_conn->exec(getCreditQuery);
    if (getCreditRes.size() == 0) {
        LOGGER(PACKAGE).error("Cannot get registrar credit from database");
        return Database::Money();
    }
    Database::Money retval;
    retval = *(*getCreditRes.begin()).begin();
    return retval;
} // ManagerImpl::getCreditByZone()

void
ManagerImpl::archiveInvoices(bool send) const
{
    LOGGER(PACKAGE).info(
            "Register::Invoicing::ManagerImpl::archiveInvoices(bool)");
    try {
        // archive unarchived invoices
        ExporterArchiver exporter(m_docMan);
        ListImpl list(m_dbMan, m_conn, (ManagerImpl *)this);
        Database::Filters::Invoice *invoiceFilter =
            new Database::Filters::InvoiceImpl();
        Database::Filters::Union *unionFilter =
            new Database::Filters::Union();
        invoiceFilter->addFilePDF().setNULL();
        unionFilter->addFilter(invoiceFilter);
        list.reload(*unionFilter);
        list.doExport(&exporter);
        if (send) {
            Mails mail(m_mailMan, m_dbMan);
            mail.load();
            mail.send();
        }
    }
    catch (...) {
        LOGGER(PACKAGE).error("Register::ManagerImpl::archiveInvoices(bool) error");
    }
} // ManagerImpl::archiveInvoices()

void
ManagerImpl::initVatList()
{
    Database::SelectQuery query;
    query.select()
        << "vat, 10000*koef, valid_to";
    query.from()
        << "price_vat";
    Database::Result res = m_conn->exec(query);
    Database::Result::Iterator it = res.begin();
    for (; it != res.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        unsigned int        vatRate     = *(col);
        unsigned int        koef        = *(++col);
        Database::DateTime  validity    = *(++col);
        m_vatList.push_back(VAT(vatRate, koef, validity));
    }
} // ManagerImpl::initVatList()

const VAT *
ManagerImpl::getVat(unsigned int rate) const
{
    // late initialization would brake constness
    if (m_vatList.size() == 0) {
        ((ManagerImpl *)this)->initVatList();
    }
    std::vector<VAT>::const_iterator it =
        find(m_vatList.begin(), m_vatList.end(), rate);
    return it == m_vatList.end() ? NULL : &(*it);
} // ManagerImpl::getVat()

Database::Money 
ManagerImpl::countVat(Database::Money price, unsigned int vatRate, bool base)
{
    const VAT *v = getVat(vatRate);
    unsigned koef = v ? v->getKoef() : 0;
    return price * koef / (10000 - (base ? koef : 0));
}

} // namespace Invoicing
} // namespace Register
