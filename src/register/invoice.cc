#include "invoice.h"
#include "invoice_manager.h"
#include "model_invoiceobjectregistry.h"
#include "model_invoiceobjectregistrypricemap.h"
#include "model_invoicecreditpaymentmap.h"
#include "model_invoicegeneration.h"
#include "db/psql/psql_connection.h"

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

//-----------------------------------------------------------------------------
//
// Invoice
//
//-----------------------------------------------------------------------------

Invoice::~Invoice()
{
    m_sources.clear();
    m_actions.clear();
}

Subject Invoice::m_supplier(0, "REG-CZNIC", "CZ.NIC, z.s.p.o.",
        "CZ.NIC, zájmové sdružení právnických osob", "Americká 23", 
        "Praha 2", "120 00", "CZ", "67985726", "CZ67985726", 
        "SpZ: odb. občanskopr. agend Magist. hl. m. Prahy, č. ZS/30/3/98", 
        "CZ.NIC, z.s.p.o., Americká 23, 120 00 Praha 2", "www.nic.cz", 
        "podpora@nic.cz", "+420 222 745 111", "+420 222 745 112", 1);

const std::string &
Invoice::getVarSymbol() const
{
    return m_varSymbol;
}
void 
Invoice::setVarSymbol(const std::string &varSymbol)
{
    m_varSymbol = varSymbol;
}
const Database::DateInterval &
Invoice::getAccountPeriod() const
{
    return m_accountPeriod;
}
void 
Invoice::setAccountPeriod(const Database::DateInterval &period)
{
    m_accountPeriod = period;
}
void 
Invoice::setType(Type type)
{
    m_type = type;
}
Type
Invoice::getType() const
{
    return m_type;
}
Database::Date 
Invoice::getToDate() const
{
    return m_toDate;
}
Database::Date 
Invoice::getFromDate() const
{
    return m_fromDate;
}
void 
Invoice::setToDate(const std::string &toDate)
{
    m_toDate = Database::Date(toDate);
}
void 
Invoice::setToDate(const Database::Date &toDate)
{
    m_toDate = toDate;
}
void
Invoice::setTaxDate(const Database::Date &taxDate)
{
    ModelInvoice::setTaxDate(taxDate);
}
void
Invoice::setTaxDate(const std::string &taxDate)
{
    ModelInvoice::setTaxDate(Database::Date(taxDate));
}
void 
Invoice::setFromDate(const std::string &fromDate)
{
    m_fromDate = Database::Date(fromDate);
}
void 
Invoice::setFromDate(const Database::Date &fromDate)
{
    m_fromDate = fromDate;
}
void 
Invoice::addError(const std::string &str)
{
    m_errors.push_back(str);
}
void 
Invoice::addError(const boost::format &frmt)
{
    m_errors.push_back(frmt.str());
}

std::string
Invoice::getLastError() const
{
    return (m_errors.size() == 0) ? "" : m_errors.back();
}
std::vector<std::string> 
Invoice::getErrors() const 
{
    return m_errors;
}
void 
Invoice::addSource(PaymentSource *source)
{
    TRACE("[CALL] Register::Invoicing::Invoice::"
            "addSource(PaymentSource *)");
    m_sources.push_back(source);

    std::vector<Payment>::iterator it =
        find(m_paid.begin(), m_paid.end(), source->getVatRate());
    if (it != m_paid.end()) {
        it->add(source);
    } else {
        m_paid.push_back(Payment(source));
    }
} // Invoice::addSource()
void
Invoice::addAction(PaymentAction *action)
{
    m_actions.push_back(action);
    m_annualPartitioning.addAction(m_actions.back());
}
void 
Invoice::clearActions()
{
    m_actions.clear();
}
unsigned int 
Invoice::getSourceCount() const 
{
    return m_sources.size();
}
const PaymentSource *
Invoice::getSource(const unsigned int &index) const
{
    return (index >= m_sources.size()) ? NULL : m_sources.at(index);
}
unsigned int 
Invoice::getActionCount() const 
{
    return m_actions.size();
}
const PaymentAction *
Invoice::getAction(const unsigned int &index) const
{
    return (index >= m_actions.size()) ? NULL : m_actions.at(index);
}
AnnualPartitioning *
Invoice::getAnnualPartitioning()
{
    return &m_annualPartitioning;
}
unsigned int 
Invoice::getPaymentCount() const
{
    if (m_type == IT_DEPOSIT && !m_paid.size()) {
        Payment payment;
        payment.setPrice(getTotal());
        payment.setVatRate(getVat());
        payment.setVat(getTotalVat());
        ((Invoice *)this)->m_paid.push_back(payment);
    }
    return m_paid.size();
} // Invoice::getPaymentCount()
const Payment *
Invoice::getPayment(const unsigned int &index) const
{
    return (index >= m_paid.size()) ? NULL : &m_paid.at(index);
}
const Subject *
Invoice::getClient() const
{
    return &m_client;
}
void 
Invoice::setClient(Subject &client)
{
    m_client = dynamic_cast<Subject &>(client);
}
const Subject *
Invoice::getSupplier() const 
{
    return &m_supplier;
}

void 
Invoice::setFile(const Database::ID &filePDF, const Database::ID &fileXML)
{
    TRACE("[CALL] Register::Invoicing::Invoice::setFile("
            "const Database::ID &, const Database::ID &");
    setFileId(filePDF);
    setFileXmlId(fileXML);
}

void
Invoice::doExport(Exporter *exp)
{
    TRACE("[CALL] Register::Invoicing::Invoice::doExport(Exporter *exp)");
    exp->doExport(this);
}

/*! get operation (domain create or renew) price from database. */
Database::Money
Invoice::getPrice2(
        Database::PSQLConnection *conn, 
        PaymentActionType operation,
        const int &period)
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "getPrice2(PaymentActionType, int)");
    Database::SelectQuery priceQuery;
    if (period > 0) {
        priceQuery.buffer() << "SELECT price, period";
    } else {
        priceQuery.buffer() << "SELECT price";
    }
    priceQuery.buffer()
        << " FROM price_list WHERE valid_from < 'now()' and "
        << "(valid_to is NULL or valid_to > 'now()')  and "
        << "operation = " << PaymentActionType2SqlType(operation)
        << " and zone = " << Database::Value(getZoneId());
    try {
        Database::Result priceResult = conn->exec(priceQuery);
        if (priceResult.size() == 0) {
            LOGGER(PACKAGE).warning(boost::format("price for zone id=%1% and "
                        "operation id=%2% not set in database => price = 0")
                        % PaymentActionType2SqlType(operation)
                        % getZoneId());
            return 0;
        }
        Database::Money money = priceResult[0]["price"];
        if (period > 0) {
            int pper = priceResult[0]["period"];
            if (pper > 0) {
                return period * money / pper;
            }
            else {
                return 0;
            }
        } else {
            return money;
        }
    } catch (...) {
        ERROR("getPrice2: An error occured");
        return Database::Money();
    }
    return Database::Money();
} // InvoiceImpl::getPrice()

/*! return true is registrar is system */
bool
Invoice::isRegistrarSystem(
        Database::PSQLConnection *conn,
        const Database::ID &registrar)
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "isRegistrarSystem(Database::ID)");
    Database::SelectQuery regQuery;
    regQuery.buffer()
        << "SELECT system FROM registrar WHERE "
        << "id = " << Database::Value(registrar);
    try {
        Database::Result regResult = conn->exec(regQuery);
        if (regResult.size() == 0) {
            ERROR("Cannot find out if registrar is system");
            return false;
        }
        std::string system = regResult[0][0];
        if (system == "t") {
            return true;
        } else {
            return false;
        }
    } catch (...) {
        ERROR("isRegistrarSystem: An error has occured");
        return false;
    }
} // InvoiceImpl::isRegistrarSystem()

std::vector<Database::Money>
Invoice::countPayments( 
        const std::vector<Database::Money> &credit,
        const Database::Money &price)
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::countPayments("
            "std::vector<Database::Money>, std::vector<Database::Money>)");
    std::vector<Database::Money> payments;
    Database::Money remainder;
    Database::Money p_price;
    for (int i = 0; i < (int)credit.size(); i++) {
        if (price > credit[i]) {
            p_price = price - credit[i];
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
Database::ID
Invoice::newInvoiceObjectRegistry(Database::PSQLConnection *conn)
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "newInvoiceObjectRegistry()");
    Database::InsertQuery query("invoice_object_registry");
    query.add("objectid", Database::Value(getAction(0)->getObjectId()));
    query.add("zone", Database::Value(getZoneId()));
    query.add("registrarid", Database::Value(getRegistrarId()));
    query.add("operation", Database::Value(getAction(0)->getAction()));
    if (getAction(0)->getExDate() != Database::Date()) {
        query.add("exdate", Database::Value(getAction(0)->getExDate()));
        query.add("period", Database::Value(getAction(0)->getUnitsCount()));
    }
    try {
        conn->exec(query);
    } catch (...) {
        ERROR("failed to insert new record into `invoice_object_registry` table");
        return Database::ID();
    }
    Database::Query selQuery;
    selQuery.buffer()
        << "SELECT currval('invoice_object_registry_id_seq')";
    try {
        Database::Result res = conn->exec(selQuery);
        Database::ID id = res[0][0];
        TRACE(boost::format("invoice_object_registry id: %1%") % id);
        return id;
    } catch (...) {
        ERROR("newInvoiceObjectRegistry: An error has occored");
        return Database::ID();
    }
    return Database::ID();
} // InvoiceImpl::newInvoiceObjectRegistry()

/*! put new line into ``invoice_object_registry_price_map'', where
 * ``invObjRegId'' is id from table ``invoice_object_registry''
 */
bool 
Invoice::newInvoiceObjectRegistryPriceMap( 
        Database::PSQLConnection *conn,
        const Database::ID &invObjRegId,
        const Database::ID &invoiceId,
        const Database::Money &price)
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "newInvoiceObjectRegistryPriceMap(Database::ID)");
    Database::InsertQuery insert("invoice_object_registry_price_map");
    insert.add("id", Database::Value(invObjRegId));
    insert.add("invoiceid", Database::Value(invoiceId));
    insert.add("price", Database::Value(price));
    try {
        conn->exec(insert);
    } catch (...) {
        ERROR("failed to insert new record into "
                "`invoice_object_registry_price_map` table");
        return false;
    }
    return true;
} // InvoiceImpl::newInvoiceObjectRegistryPriceMap()

bool
Invoice::updateInvoiceCredit(
        Database::PSQLConnection *conn,
        const Database::ID &invoiceId,
        const Database::Money &price)
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "updateInvoiceCredit(Database::ID, Database::Money)");
    Database::Query query;
    query.buffer()
        << "UPDATE invoice SET credit=credit-" << Database::Value(price)
        << " WHERE id=" << Database::Value(invoiceId);
    try {
        conn->exec(query);
    } catch (...) {
        ERROR("failed to update `credit` in the `invoice` table");
        return false;
    }
    return true;
} // InvoiceImpl::updateInvoiceCredit()

bool
Invoice::getCreditInvoicesId(
        Database::PSQLConnection *conn,
        std::vector<Database::ID> &vec_id,
        std::vector<Database::Money> &vec_money,
        const int &limit)
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "getCreditInvoices(int)");
    std::vector<Database::ID> ret;
    Database::Query selectQuery;
    selectQuery.buffer()
        << "SELECT id, credit FROM invoice WHERE "
        << "registrarid = " << Database::Value(getRegistrarId())
        << " and (zone = " << Database::Value(getZoneId()) << " or zone isnull)"
        << " and credit > 0 order by zone, id limit " << limit
        << " FOR UPDATE;";
    try {
        Database::Result res = conn->exec(selectQuery);
        if (res.size() == 0) {
            ERROR("cannot get credit invoices");
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
    } catch (...) {
        ERROR("getCreditInvoicesId: An error has occured");
        return false;
    }
    return true;
} // InvoiceImpl::getCreditInvoicesId()

bool
Invoice::domainBilling(
        DB *db,
        const Database::ID &zone,
        const Database::ID &registrar,
        const Database::ID &objectId,
        const Database::Date &exDate,
        const int &units_count)
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "domainBilling(DB *, Database::ID, "
            "Database::ID, Database::Date, int)");

    PGconn *pg_conn = db->__getPGconn();
    Database::PSQLConnection *psql_conn = new Database::PSQLConnection(pg_conn);
    return domainBilling(psql_conn, zone, registrar, objectId, exDate, units_count);
}

bool 
Invoice::domainBilling(
        Database::PSQLConnection *conn,
        const Database::ID &zone,
        const Database::ID &registrar,
        const Database::ID &objectId,
        const Database::Date &exDate,
        const int &units_count)
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "domainBilling(Database::PSQLConnection *,Database::ID, "
            "Database::ID, Database::Date, int)");
    setZoneId(zone);
    setRegistrarId(registrar);
    PaymentAction *action = new PaymentAction();
    action->setAction(PAT_CREATE_DOMAIN);
    action->setObjectId(objectId);
    action->setUnitsCount(12);
    addAction(action);
    if (!domainBilling(conn)) {
        return false;
    }
    clearActions();
    PaymentAction *action_renew = new PaymentAction();
    action_renew->setAction(PAT_RENEW_DOMAIN);
    action_renew->setObjectId(objectId);
    action_renew->setExDate(exDate);
    action_renew->setUnitsCount(units_count);
    addAction(action_renew);
    if (!domainBilling(conn)) {
        return false;
    }
    
    return true;
} // InvoiceImpl::DomainBilling()

bool 
Invoice::domainBilling(Database::PSQLConnection *conn)
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::domainBilling()");
    if (isRegistrarSystem(conn, getRegistrarId())) {
        LOGGER(PACKAGE).debug(
                boost::format("This is system registrar (id: %1%)")
                % getRegistrarId());
        return true;
    }
    /* get price for desired operation */
    Database::Money price = getPrice2(
            conn, getAction(0)->getAction(), getAction(0)->getUnitsCount());

    /* zero price should be possible */
    if (price == Database::Money(0)) {
        LOGGER(PACKAGE).info("operation has zero price; billing of operation stopped");
        return true;
    }
    else {
        LOGGER(PACKAGE).info(boost::format("operation price=%1%; proceeding "
                    "with invoice") % price);
    }

    std::vector<Database::ID> vec_invoiceId;
    std::vector<Database::Money> vec_money;
    if (!getCreditInvoicesId(conn, vec_invoiceId, vec_money, 10)) {
        return false;
    }

    Database::ID invoiceObjectRegistryId = newInvoiceObjectRegistry(conn);
    if (invoiceObjectRegistryId == Database::ID()) {
        return false;
    }
    std::vector<Database::Money> vec_payments = countPayments(vec_money, price);
    if (vec_payments.empty()) {
        return false;
    }
    for (int i = 0; i < (int)vec_payments.size(); i++) {
        if (!newInvoiceObjectRegistryPriceMap(conn, invoiceObjectRegistryId,
                    vec_invoiceId[i], vec_payments[i])) {
            return false;
        }
        if (!updateInvoiceCredit(conn, vec_invoiceId[i], vec_payments[i])) {
            return false;
        }
    }
    return true;
} // InvoiceImpl::domainBilling()

/*! return proper 'from date' */
Database::Date 
Invoice::getFromDateFromDB()
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::getFromDateFromDB()");
    Database::Date fromDate;
    Database::SelectQuery fromDateQuery;
    fromDateQuery.buffer()
        << "SELECT date( todate + interval '1 day') "
        // << "SELECT todate "
        << "from invoice_generation  WHERE"
        << " zone = " << Database::Value(getZoneId())
        << " AND registrarid = " << Database::Value(getRegistrarId())
        << " order by id desc limit 1;";
    Database::Connection conn = Database::Manager::acquire();
    Database::Result fromDateRes1 = conn.exec(fromDateQuery);
    if (fromDateRes1.size() == 0) {
        Database::SelectQuery fromDateQuery2;
        fromDateQuery2.buffer()
            << "SELECT fromdate from registrarinvoice WHERE"
            << " zone = " << Database::Value(getZoneId())
            << " and registrarid = " << Database::Value(getRegistrarId());
        Database::Result fromDateRes2 = conn.exec(fromDateQuery2);
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
int 
Invoice::getRecordsCount()
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "getRecordsCount()");
    int recordsCount;
    Database::SelectQuery recordCountQuery;
    recordCountQuery.buffer()
        << "SELECT count(id) from invoice_object_registry where"
        << " crdate < " << Database::Value(getCrDate())
        << " AND  zone = " << Database::Value(getZoneId())
        << " AND registrarid = " << Database::Value(getRegistrarId())
        << " AND invoiceid IS NULL;";
    Database::Connection conn = Database::Manager::acquire();
    Database::Result recordCountRes = conn.exec(recordCountQuery);
    if (recordCountRes.size() == 0) {
        return -1;
    } else {
        recordsCount = *(*recordCountRes.begin()).begin();
    }
    return recordsCount;
} // InvoiceImpl::getRecordsCount()

/*! return price of all invoiced items */
bool 
Invoice::getRecordsPrice()
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "getRecordsPrice()");
    Database::Money recordsPrice;
    Database::SelectQuery recordsPriceQuery;
    recordsPriceQuery.buffer()
        << "SELECT CAST(sum(price) AS INTEGER) FROM invoice_object_registry, "
        << "invoice_object_registry_price_map WHERE "
        << "invoice_object_registry_price_map.id=invoice_object_registry.id"
        << " AND  crdate < " <<  Database::Value(getCrDate())
        << " AND zone = " << Database::Value(getZoneId())
        << " AND registrarid = " << Database::Value(getRegistrarId())
        << " AND  invoice_object_registry.invoiceid is null";
    Database::Connection conn = Database::Manager::acquire();
    Database::Result recordsPriceRes = conn.exec(recordsPriceQuery);
    if (recordsPriceRes.size() == 0) {
        return false;
    } else {
        recordsPrice = *(*recordsPriceRes.begin()).begin();
    }
    setPrice(recordsPrice);
    return true;
} // InvoiceImpl::getRecordsPrice()

/*! return valid vat value from ''price_vat'' table */
int 
Invoice::getSystemVAT()
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::getSystemVAT()");
    int vat;
    Database::SelectQuery vatQuery;
    vatQuery.buffer()
        << "select vat from price_vat "
        << "where valid_to > now() or valid_to is null;";
    Database::Connection conn = Database::Manager::acquire();
    Database::Result vatRes = conn.exec(vatQuery);
    if (vatRes.size() == 0) {
        return -1;
    } else {
        vat = *(*vatRes.begin()).begin();
    }
    return vat;
} // InvoiceImpl::getSystemVAT()

/*! check if table ``invoice_prefix'' contain any row with given year */
bool 
Invoice::isYearInInvoicePrefix(const int &year)
{
    LOGGER(PACKAGE).debug(
            "[CALL] Register::Invoicing::InvoiceImpl::"
            "isYearInInvoicePrefix(int)");
    Database::Query yearQuery;
    yearQuery.buffer()
        << "SELECT id FROM invoice_prefix WHERE year = " << Database::Value(year);
    Database::Connection conn = Database::Manager::acquire();
    Database::Result yearRes = conn.exec(yearQuery);
    if (yearRes.size() == 0) {
        return false;
    }
    return true;
} // InvoiceImpl::isYearInInvoicePrefix()

/*! create new rows in ``invoice_prefix'' from some
 * default values
 */
bool 
Invoice::createNewYearInInvoicePrefix(const int &newYear)
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "createNewYearInInvoicePrefix(int)");
    ModelInvoicePrefix mip1;
    ModelInvoicePrefix mip2;
    ModelInvoicePrefix mip3;
    ModelInvoicePrefix mip4;
    unsigned long long year = (newYear - 2000) * 100000;
    mip1.setZoneId(1);
    mip1.setType(0);
    mip1.setYear(newYear);
    mip1.setPrefix(110000001 + year);

    mip2.setZoneId(1);
    mip2.setType(1);
    mip2.setYear(newYear);
    mip2.setPrefix(120000001 + year);
    
    mip3.setZoneId(3);
    mip3.setType(0);
    mip3.setYear(newYear);
    mip3.setPrefix(130000001 + year);

    mip4.setZoneId(3);
    mip4.setType(1);
    mip4.setYear(newYear);
    mip4.setPrefix(140000001 + year);

    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction transaction(conn);

    try {
        mip1.insert();
        mip2.insert();
        mip3.insert();
        mip4.insert();
    } catch (...) {
        ERROR("Cannot add new rows into invoice_prefix table");
        return false;
    }
    transaction.commit();
    return true;
} // InvoiceImpl::createNewYearInInvoicePrefix()

/*! create new rows in ``invoice_prefix'' table for new year
 * according oldYear
 */
bool 
Invoice::createNewYearInInvoicePrefix(const int &newYear, const int &oldYear)
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "createNewYearInInvoicePrefix(int, int)");
    Database::Query selectQuery;
    selectQuery.buffer()
        << "SELECT zone, typ, "
        << " CAST(substr(CAST(prefix AS varchar), 0, 3) AS integer) AS num"
        << " FROM invoice_prefix"
        << " WHERE year = " << Database::Value(oldYear)
        << " ORDER BY num;";
    Database::Connection conn = Database::Manager::acquire();
    Database::Result selectRes = conn.exec(selectQuery);
    if (selectRes.size() == 0) {
        // oops, there is no year, from which i can copy
        return createNewYearInInvoicePrefix(newYear);
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

        ModelInvoicePrefix invPrefix;
        invPrefix.setZoneId(zone);
        invPrefix.setType(type);
        invPrefix.setYear(newYear);
        invPrefix.setPrefix(prefix);
        try {
            invPrefix.insert();
        } catch (...) {
            ERROR("failed to create new row in `invoice_prefix` table");
            return false;
        }
    }
    return true;
} // InvoiceImpl::createNewYearInInvoicePrefix()

/*! get and update (i.e. add 1 to it) invoice number, save 
 * invoice prefix using ``setNumber()'' method, also get
 * invoice prefix type id (and save it using
 * ``setInvoicePredixTypeId()''
 */
bool 
Invoice::getInvoicePrefix()
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "getInvoicePrefix()");
    long invoicePrefix;
    Database::ID id;
    Database::SelectQuery invoicePrefixQuery;
    invoicePrefixQuery.buffer()
        << "SELECT id, prefix FROM invoice_prefix WHERE zone";
    if (getZoneId() == Database::ID()) {
        invoicePrefixQuery.buffer()
            << " is null";
    } else {
        invoicePrefixQuery.buffer()
            << " = " << Database::Value(getZoneId());
    }
    invoicePrefixQuery.buffer()
        << " AND typ = " << Database::Value(getType())
        << " AND year = " << Database::Value(getTaxDate().get().year());
    Database::Connection conn = Database::Manager::acquire();
    Database::Result invoicePrefixRes = conn.exec(invoicePrefixQuery);
    if (invoicePrefixRes.size() == 0) {
        if (getZoneId() == Database::ID()) {
            // invoice without zone - no auto create (see if below)
            ERROR("there is no invoice prefix with empty zone");
            return false;
        }
        // try to create new prefix number(s) according rows from last year
        if (!createNewYearInInvoicePrefix(
                getTaxDate().get().year(), 
                getTaxDate().get().year() - 1)) {
            return false;
        }
        Database::Result invoicePrefixRes = conn.exec(invoicePrefixQuery);
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

    Database::UpdateQuery uquery("invoice_prefix");
    uquery.add("prefix", invoicePrefix + 1);
    uquery.where().add("id", "=", id, "AND");
    try {
        conn.exec(uquery);
    } catch (...) {
        ERROR("cannot update `invoice_prefix` table");
        return false;
    }
    setPrefix(invoicePrefix);
    setPrefixTypeId(id);
    return true;
} // InvoiceImpl::getInvoicePrefix()

bool
Invoice::createNewInvoice()
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::createNewInvoice()");
    if (!getInvoicePrefix()) {
        return false;
    }
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction transaction(conn);


    try {
        insert();
    } catch (...) {
        ERROR("cannot insert new row into `invoice` table");
        return false;
    }

    ModelInvoiceGeneration invoiceGeneration;
    invoiceGeneration.setFromDate(getFromDate());
    invoiceGeneration.setToDate(getToDate());
    invoiceGeneration.setRegistrarId(getRegistrarId());
    invoiceGeneration.setZoneId(getZoneId());
    invoiceGeneration.setInvoiceId(getId());

    try {
        invoiceGeneration.insert();
    } catch (...) {
        ERROR("cannot insert new row into `invoice_generation` table");
        return false;
    }
    transaction.commit();
    return true;
}

/*! update record(s) in ''invoice_object_registry'' table - set id */
bool 
Invoice::updateInvoiceObjectRegistry()
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "updateInvoiceObjectRegistry()");
    Database::UpdateQuery uquery("invoice_object_registry");
    uquery.add("invoiceid", getId());
    uquery.where().add("crdate", "<", getCrDate(), "AND");
    uquery.where().add("zone", "=", getZoneId(), "AND");
    uquery.where().add("registrarid", "=", getRegistrarId(), "AND");
    uquery.where().add("invoiceid", "IS", Database::Value(), "AND");
    Database::Connection conn = Database::Manager::acquire();
    try {
        conn.exec(uquery);
    } catch (...) {
        ERROR("cannot update record in `invoice_object_registry`");
        return false;
    }
    return true;
} // InvoiceImpl::updateInvoiceObjectRegistry()

/*! update record(s) in ''registrarinvoice'' table - set lastdate */
bool 
Invoice::updateRegistrarInvoice()
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "updateRegistrarInvoice()");
    Database::UpdateQuery uquery("registrarinvoice");
    uquery.add("lastdate", getToDate());
    uquery.where().add("zone", "=", getZoneId(), "AND");
    uquery.where().add("registrarid", "=", getRegistrarId(), "AND");
    Database::Connection conn = Database::Manager::acquire();
    try {
        conn.exec(uquery);
    } catch (...) {
        ERROR("cannot update record in `registrarinvoice`");
        return false;
    }
    return true;
} // InvoiceImpl::updateRegistrarInvoice()

/*! return numbers of account invoices from which are object payed */
std::vector<int> 
Invoice::getAccountInvoicesNumbers()
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "getAccountInvoicesNumbers()");
    std::vector<int> ret;
    Database::SelectQuery getInvoicesNumbersQuery;
    getInvoicesNumbersQuery.buffer()
        << "SELECT invoice_object_registry_price_map.invoiceid FROM"
        << " invoice_object_registry, invoice_object_registry_price_map"
        << " WHERE invoice_object_registry.id ="
        << " invoice_object_registry_price_map.id AND"
        << " invoice_object_registry.invoiceid = " << Database::Value(getId())
        << " GROUP BY invoice_object_registry_price_map.invoiceid" ;
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec(getInvoicesNumbersQuery);
    Database::Result::Iterator it = res.begin();
    for (;it != res.end(); ++it) {
        int id = *(*it).begin();
        ret.push_back(id);
    }
    return ret;
} // InvoiceImpl::getAccountInvoicesNumbers()

Database::Money 
Invoice::getInvoiceSumPrice(const int &accountInvoiceId)
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::" 
            "getInvoiceSumPrice(int)");
    Database::Money sumPrice;
    Database::SelectQuery sumPriceQuery;
    sumPriceQuery.buffer()
        << "SELECT SUM(invoice_object_registry_price_map.price) "
        << " FROM invoice_object_registry, invoice_object_registry_price_map"
        << " WHERE invoice_object_registry.id ="
        << " invoice_object_registry_price_map.id"
        << " AND invoice_object_registry.invoiceid = " << Database::Value(getId())
        << " AND invoice_object_registry_price_map.invoiceid = "
        << Database::Value(accountInvoiceId);
    Database::Connection conn = Database::Manager::acquire();
    Database::Result sumPriceRes = conn.exec(sumPriceQuery);
    if (sumPriceRes.size() == 0) {
        return -1;
    } else {
        sumPrice = *(*sumPriceRes.begin()).begin();
    }
    return sumPrice;
} // InvoiceImpl::getInvoiceSumPrice()

Database::Money 
Invoice::getInvoiceBalance(
        const int &accountInvoiceId,
        const Database::Money &invoiceSumPrice)
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::"
            "getInvoiceBalance(int, long)");
    TRACE(boost::format("invoiceSumPrice=%1%") % invoiceSumPrice);
    Database::SelectQuery totalQuery;
    totalQuery.buffer()
        << "SELECT total FROM invoice"
        << " WHERE id = "<< Database::Value(accountInvoiceId);
    Database::Connection conn = Database::Manager::acquire();
    Database::Result totalResult = conn.exec(totalQuery);
    Database::Money total;
    if (totalResult.size() == 0) {
        return -1;
    } else {
        total = *(*totalResult.begin()).begin();
    }
    Database::SelectQuery sumQuery;
    sumQuery.buffer()
        << "SELECT CAST(SUM(credit) AS INTEGER)"
        << " FROM invoice_credit_payment_map"
        << " WHERE ainvoiceid = " << Database::Value(accountInvoiceId);
    Database::Result sumResult = conn.exec(sumQuery);
    Database::Money sum;
    if (sumResult.size() == 0) {
        return -1;
    } else {
        sum = *(*sumResult.begin()).begin();
    }
    Database::Money price = total - sum - invoiceSumPrice;
    // price = price - sum;
    // price = price - invoiceSumPrice;
    return price;
} // InvoiceImpl::getInvoiceBalance()

bool 
Invoice::updateInvoiceCreditPaymentMap(const std::vector<int> &idNumbers)
{
    LOGGER(PACKAGE).debug(
            "[CALL] Register::Invoicing::InvoiceImpl::"
            "updateInvoiceCreditPaymentMap(std::vector<int>)");
    Database::Connection conn = Database::Manager::acquire();
    for (int i = 0; i < (int)idNumbers.size(); i++) {
        // long invoiceSumPrice = getInvoiceSumPrice(idNumbers[i]);
        // long invoiceBalance = getInvoiceBalance(idNumbers[i], invoiceSumPrice);
        Database::Money invoiceSumPrice = getInvoiceSumPrice(idNumbers[i]);
        Database::Money  invoiceBalance = getInvoiceBalance(idNumbers[i], invoiceSumPrice);

        ModelInvoiceCreditPaymentMap micpm;
        micpm.setInvoiceId(getId());
        micpm.setAdvanceInvoiceId(idNumbers[i]);
        micpm.setCredit(invoiceSumPrice);
        micpm.setBalance(invoiceBalance);
        try {
            micpm.insert();
        } catch (...) {
            ERROR("cannot insert row into `invoice_credit_payment_map`");
            return false;
        }
    }
    return true;
} // InvoiceImpl::updateInvoiceCreditPaymentMap()

bool 
Invoice::testRegistrar()
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::testRegistrar()");
    Database::Connection conn = Database::Manager::acquire();
    if (getRegistrarId() == Database::ID() && getRegistrar()->getHandle().empty()) {
        ERROR("Registrar not set");
        return false;
    } else if (getRegistrarId() == Database::ID()) {
        Database::SelectQuery regQuery;
        regQuery.buffer()
            << "select id from registrar where handle = "
            << Database::Value(getRegistrar()->getHandle());
        Database::Result res = conn.exec(regQuery);
        if (res.size() == 0) {
            ERROR("registrar do not exists");
            return false;
        } else {
            setRegistrarId(*(*res.begin()).begin());
        }
    } else if (getRegistrar()->getHandle().empty()) {
        Database::SelectQuery query;
        query.buffer()
            << "select id from registrar where id = "
            << Database::Value(getRegistrarId());
        Database::Result res = conn.exec(query);
        if (res.size() == 0) {
            ERROR("registrar do not exists");
            return false;
        }
    } else {
        Database::SelectQuery query;
        query.buffer()
            << "select id from registrar where"
            << " handle = " << Database::Value(getRegistrar()->getHandle())
            << " and id = " << Database::Value(getRegistrarId());
        Database::Result res = conn.exec(query);
        if (res.size() == 0) {
            ERROR("clash between registrar id and handle");
            return false;
        }
    }
    return true;
} // InvoiceImpl::testRegistrar()

bool
Invoice::testRegistrarPrivileges()
{
    TRACE("[CALL] Register::Invoicing::Invoice::testRegistrarPrivileges()");
    Database::Connection conn = Database::Manager::acquire();
    if (getRegistrarId() == Database::ID()) {
        ERROR("Registrar id not set");
        return false;
    }
    Database::Query query;
    query.buffer()
        << "SELECT registrarid FROM registrarinvoice "
        << "WHERE registrarid=" << Database::Value(getRegistrarId())
        << " AND fromdate<=date(now())";
    try {
        Database::Result res = conn.exec(query);
        if (res.size() == 0) {
            return false;
        }
    } catch (...) {
        ERROR("An exception was catched");
        return false;
    }
    return true;
} // Invoice::testRegistrarPrivileges

bool 
Invoice::testZone()
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::testZone()");
    Database::Connection conn = Database::Manager::acquire();
    if (getZoneId() == 0 && getZone()->getFqdn().empty()) {
        ERROR("Zone not set");
        return false;
    } else if (getZoneId() == 0) {
        Database::SelectQuery zoneQuery;
        zoneQuery.buffer()
            << "select zz.id from zone zz "
            << "join registrarinvoice rr on (rr.zone = zz.id)"
            << " where zz.fqdn = " << Database::Value(getZone()->getFqdn())
            << " and rr.registrarid = " << Database::Value(getRegistrarId())
            << " and rr.fromdate <= date(now())"
            << " limit 1";
        Database::Result res = conn.exec(zoneQuery);
        if (res.size() == 0) {
            ERROR(boost::format("registrar (id:%1%) do not belong to zone")
                    % getRegistrarId());
            return false;
        } else {
            setZoneId(*(*res.begin()).begin());
        }
    } else if (getZone()->getFqdn().empty()) {
        Database::SelectQuery zoneQuery;
        zoneQuery.buffer()
            << "select zz.id from zone zz"
            << " join registrarinvoice rr on (rr.zone = zz.id)"
            << " where zz.id = " << Database::Value(getZoneId())
            << " and rr.registrarid = " << Database::Value(getRegistrarId())
            << " and rr.fromdate <= date(now()) "
            << " limit 1";
        Database::Result res = conn.exec(zoneQuery);
        if (res.size() == 0) {
            ERROR(boost::format("registrar (id:%1%) do not belong to zone")
                    % getRegistrarId());
            return false;
        }
    } else {
        Database::SelectQuery zoneQuery;
        zoneQuery.buffer()
            << "select zz.id from zone zz"
            << " join registrarinvoice rr on (rr.zone = zz.id)"
            << " where zz.id = " << Database::Value(getZoneId())
            << " and zz.fqdn = " << Database::Value(getZone()->getFqdn())
            << " and rr.registrarid = " << Database::Value(getRegistrarId())
            << " and rr.fromdate < date(now()) limit 1";
        Database::Result res = conn.exec(zoneQuery);
        if (res.size() == 0) {
            ERROR("zone name does not correspond to zone id");
            return false;
        }
    }
    return true;
} // InvoiceImpl::testZone()

/*! create new account invoice */
bool 
Invoice::insertAccount()
{
    TRACE("[CALL] Register::Invoicing::Invoice::insertAccount()");
    
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction transaction(conn);

    if (!testRegistrar()) {
        return false;
    }
    if (!testRegistrarPrivileges()) {
        LOGGER(PACKAGE).warning("registrar hasn't privileges for this zone");
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
    setCrDate(Database::DateTime(Database::NOW));
    int recordsCount = getRecordsCount();
    if (recordsCount == -1) {
        ERROR("cannot get count of records to invoicing");
        return false;
    }
    if (recordsCount == 0) {
        LOGGER(PACKAGE).warning(boost::format("%1%: no records to invoice")
                % getRegistrar()->getHandle());
        return true;
    }
    if (!getRecordsPrice()) {
        ERROR("cannot get price of records to invoicing");
        return false;
    }
    LOGGER(PACKAGE).debug(boost::format(
                "invoicing from %1% to %2%, tax date %3%, "
                "create timestamp %4%, price %5%")
            % fromDate % getToDate() % getTaxDate() % getCrDate()
            % getPrice());

    if (!createNewInvoice()) {
        ERROR("cannot create new invoice");
        return false;
    }
    if (recordsCount > 0) {
        if (!updateInvoiceObjectRegistry()) {
            return false;
        }
    }
    if (!updateRegistrarInvoice()) {
        return false;
    }
    std::vector<int> numbers = getAccountInvoicesNumbers();
    if (numbers.size() == 0) {
        return false;
    }
    if (!updateInvoiceCreditPaymentMap(numbers)) {
        return false;
    }

    transaction.commit();
    return true;
} // InvoiceImpl::insertAccount()

bool 
Invoice::insertDeposit()
{
    TRACE("[CALL] Register::Invoicing::InvoiceImpl::insertDeposit()");

    if (!testRegistrar()) {
        return false;
    }
    if (!testRegistrarPrivileges()) {
        ERROR("registrar hasn't privileges for this zone");
        return false;
    }
    // XXX this allow insert deposit invoice without zone
    if (getZoneId() != 0 || getZone() != NULL) {
        if (!getZone()->getFqdn().empty()) {
            if (!testZone()) {
                return false;
            }
        }
    }

    if (getVat() == 0) {
        Database::SelectQuery vatQuery;
        vatQuery.buffer()
            << "select pv.vat from price_vat pv, registrar r "
            << "where r.vat=true and pv.valid_to isnull and "
            << "r.id = " << Database::Value(m_registrarId) << " limit 1";
        Database::Connection conn = Database::Manager::acquire();
        Database::Result res = conn.exec(vatQuery);
        if (res.size() != 0) {
            // get vat rate from database
            setVat(res[0][0]);
        } else {
            // XXX default czech value
            setVat(19);
        }
    }
    if (getCrDate() == Database::DateTime()) {
        setCrDate(Database::NOW);
    }
    if (getTaxDate() == Database::Date()) {
        setTaxDate(Database::NOW);
    }

    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction transaction(conn);

    if (getPrefix() == 0) {
        if (!getInvoicePrefix()) {
            ERROR("failed to obtain invoice prefix");
            return false;
        }
    }

    setTotalVat(m_manager->countVat(getPrice(), getVat(), false));
    setCredit(getPrice() - getTotalVat());
    setTotal(getCredit());
    
    try {
        insert();
    } catch (...) {
        ERROR("Failed to insert a new invoice.");
        return false;
    }
    transaction.commit();

    return true;
} // InvoiceImpl::insertDeposit()

bool 
Invoice::updateAccount()
{
    TRACE("[CALL] Register::Invoicing::Invoice::updateAccount()");
    LOGGER(PACKAGE).debug("not implemented");
    return false;
}

bool 
Invoice::updateDeposit()
{
    TRACE("[CALL] Register::Invoicing::Invoice::updateDeposit()");
    LOGGER(PACKAGE).debug("not implemented");
    return false;
}

#if 0
bool 
Invoice::update()
{
    TRACE("[CALL] Register::Invoicing::Invoice::update()");
    Database::UpdateQuery uquery("invoice");
    uquery.add("zone", getZone());
    uquery.add("crdate", getCrTime());
    uquery.add("taxdate", getTaxDate());
    uquery.add("prefix", getNumber());
    uquery.add("registrarid", getRegistrar());
    uquery.add("credit", getCredit());
    uquery.add("price",  getPrice());
    uquery.add("vat", getVatRate());
    uquery.add("total", getTotal());
    uquery.add("totalvat", getTotalVAT());
    uquery.add("prefix_type", getInvoicePrefixTypeId());
    uquery.add("file", getFilePDF());
    uquery.add("filexml", getFileXML());
    uquery.where().add("id", "=", id_, "AND");
    Database::Connection conn = Database::Manager::acquire();
    try {
        conn.exec(uquery);
    } catch (...) {
        ERROR("failed to update record in the `invoice` table");
        return false;
    }
    return true;
} // InvoiceImpl::update()
#endif

bool 
Invoice::save() 
{
    TRACE("[CALL] Register::Invoicing::Invoice::save()");
    if (getId()) {
        update();
    } else {
        if (getType() == IT_ACCOUNT) {
            return insertAccount();
        } else if (getType() == IT_DEPOSIT) {
            return insertDeposit();
        }
    }
    return true;
}

} // namespace Invoicing
} // namespace Register

