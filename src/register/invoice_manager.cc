#include "invoice_manager.h"
#include "invoice_exporter.h"
#include "invoice_mails.h"

namespace Register {
namespace Invoicing {

//-----------------------------------------------------------------------------
//
// Manager
//
//-----------------------------------------------------------------------------

List *
Manager::createList() const
{
    return new List((Manager *)this);
    // return new List();
}
Invoice *
Manager::createInvoice(Type type) const
{
    TRACE("[CALL] Register::Invoicing::Manager::createInvoice(Type)");
    Invoice *invoice = new Invoice((Manager *)this);
    invoice->setType(type);
    return invoice;
}
Invoice *
Manager::createAccountInvoice() const
{
    return createInvoice(IT_ACCOUNT);
}

// aka credit invoice
Invoice *
Manager::createDepositInvoice() const
{
    return createInvoice(IT_DEPOSIT);
}

bool
Manager::hasStatementAnInvoice(const Database::ID &statementId)
{
    TRACE("[CALL] Register::Invoicing::Manager::hasStatementAnInvoice()");
    Database::Query query;
    query.buffer()
        << "SELECT invoice_id FROM bank_item WHERE id="
        << Database::Value(statementId);
    Database::Connection conn = Database::Manager::acquire();
    try {
        Database::Result res = conn.exec(query);
        Database::ID invoiceId = res[0][0];
        if (invoiceId != Database::ID()) {
            LOGGER(PACKAGE).error(boost::format(
                        "This payment (id: %1%) already has invoice (id: %2%)")
                    % statementId % invoiceId);
            return true;
        }
    } catch (...) {
        LOGGER(PACKAGE).error("An error has occured");
        return true;
    }
    return false;
}

bool
Manager::manualCreateInvoice(
        const Database::ID &paymentId,
        const std::string &registrarHandle)
{
    Database::Query query;
    query.buffer()
        << "SELECT id FROM registrar WHERE handle="
        << Database::Value(registrarHandle);
    Database::Connection conn = Database::Manager::acquire();
    Database::ID registrarId;
    try {
        Database::Result res = conn.exec(query);
        registrarId = res[0][0];
    } catch (...) {
        LOGGER(PACKAGE).error("An error has occured");
        return false;
    }
    return manualCreateInvoice(paymentId, registrarId);
}

bool
Manager::manualCreateInvoice(
        const Database::ID &paymentId,
        const Database::ID &registrarId)
{
    TRACE("[CALL] Register::Invoicing::Manager::manualCreateInvoice("
        "const Database::ID &, const Database::ID &)");
    if (hasStatementAnInvoice(paymentId)) {
        return false;
    }
    Database::Query query;
    query.buffer()
        << "SELECT ba.zone, bi.price, bi.account_date"
        << " FROM bank_item bi"
//        << " JOIN bank_head bh ON bi.statement_id=bh.id"
        << " JOIN bank_account ba ON bi.account_id=ba.id"
        << " WHERE bi.id="
        << Database::ID(paymentId);
    Database::ID zoneId;
    Database::Money price;
    Database::Date date;
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction transaction(conn);
    try {
        Database::Result res = transaction.exec(query);
        if (res.size() == 0) {
            LOGGER(PACKAGE).error("No result");
            return false;
        }
        zoneId = res[0][0];
        price = res[0][1];
        date = res[0][2];
    } catch (...) {
        LOGGER(PACKAGE).error("An error has occured");
        return false;
    }
    std::auto_ptr<Invoice> invoice(createDepositInvoice());
    invoice->setZoneId(zoneId);
    invoice->setRegistrarId(registrarId);
    invoice->setPrice(price);
    invoice->setTaxDate(date);
    bool retval = invoice->save();
    if (!retval) {
        LOGGER(PACKAGE).warning("Failed to save credit invoice");
        return false;
    }
    retval = setInvoiceToStatementItem(paymentId, invoice->getId());
    if (!retval) {
        LOGGER(PACKAGE).error("Unable to set invoice to statement item.");
        return false;
    }
    transaction.commit();
    return true;
} // Manager::manualCreateInvoice

bool
Manager::setInvoiceToStatementItem(
        Database::ID statementId,
        Database::ID invoiceId)
{
    TRACE("[CALL] Register::Invoicing::Manager::setInvoiceToStatementItem()");
    Database::Query update;
    update.buffer()
        << "UPDATE bank_item SET invoice_id="
        << Database::Value(invoiceId)
        << " WHERE id=" << Database::Value(statementId);
    Database::Connection conn = Database::Manager::acquire();
    try {
        conn.exec(update);
    } catch (...) {
        return false;
    }
    return true;
}

bool
Manager::pairCreditInvoices(bool report)
{
    TRACE("[CALL] Register::Invoicing::Manager::pairCreditInvoices()");
    Database::Query query;
    query.buffer()
        << "SELECT bi.id, ba.zone, rr.id, bi.price, bi.account_date"
        << " FROM bank_item bi"
//        << " JOIN bank_head bh ON bi.statement_id=bh.id"
        << " JOIN bank_account ba ON bi.account_id=ba.id"
        << " JOIN registrar rr ON bi.varsymb=rr.varsymb"
        << " OR (length(trim(rr.regex)) > 0 and bi.account_memo ~* trim(rr.regex))"
        << " WHERE bi.invoice_id IS NULL AND bi.code=2 AND bi.type=1;";
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction transaction(conn);
    Database::Result res = transaction.exec(query);
    Database::Result::Iterator it = res.begin();
    for (; it != res.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();
        std::auto_ptr<Invoice> invoice(createDepositInvoice());
        Database::ID statementId = *(col);
        Database::ID zoneId = *(++col);
        Database::ID registrarId = *(++col);
        Database::Money price = *(++col);
        Database::Date date = *(++col);
        invoice->setZoneId(zoneId);
        invoice->setRegistrarId(registrarId);
        invoice->setPrice(price);
        invoice->setTaxDate(date);
        bool retval = invoice->save();
        if (!retval) {
            LOGGER(PACKAGE).warning("Failed to save credit invoice");
            continue;
        }
        retval = setInvoiceToStatementItem(statementId, invoice->getId());
        if (!retval) {
            LOGGER(PACKAGE).error("Unable to update bank_item table");
            return false;
        }
    }
    transaction.commit();
    if (report) {
        Database::Query query;
        query.buffer()
            << "SELECT id, account_date, account_name FROM bank_item "
            << "WHERE invoice_id IS NULL AND code=2 AND type=1;";
        Database::Result res = conn.exec(query);
        for (int i = 0; i < (int)res.size(); i++) {
            Database::ID id = res[i][0];
            Database::Date date = res[i][1];
            std::string name = res[i][2];
            std::cerr << id << ": " << date << " - " << name << std::endl;
        }
    }
    return true;
} // Manager::pairCreditInvoices
bool
Manager::pairAccountInvoices(bool report)
{
    TRACE("[CALL] Register::Invoicing::Manager::pairAccountInvoices()");
    return true;
}
bool
Manager::pairInvoices(bool report)
{
    TRACE("[CALL] Register::Invoicing::Manager::pairInvoices()");
    if (!pairCreditInvoices(report)) {
        return false;
    }
    if (!pairAccountInvoices(report)) {
        return false;
    }
    return true;
}
Manager *
Manager::create(Document::Manager *docMan, Mailer::Manager *mailMan)
{
    TRACE("[CALL] Register::Invoicing::Manager::create("
        "Document::Manager *, Mailer::Manager *)");
    return new Manager(docMan, mailMan);
}
Manager *
Manager::create()
{
    TRACE("[CALL] Register::Invoicing::Manager::create()");
    return new Manager();
}

Database::Money
Manager::getCreditByZone(
        const std::string &registrarHandle,
        const unsigned long long &zoneId) const
{
    TRACE("[CALL] Register::Invoicing::Manager::getCreditByZone("
            "const std::string &, Database::ID)");
    Database::SelectQuery getCreditQuery;
    getCreditQuery.buffer()
        << "SELECT SUM(credit) FROM invoice i JOIN registrar r "
        << "ON (i.registrarid=r.id) ";
    if (zoneId == 0) {
        getCreditQuery.buffer() 
            << "WHERE i.zone IS NULL ";
    } else {
        getCreditQuery.buffer() 
            << "WHERE i.zone = " << Database::Value(zoneId);
    }
    getCreditQuery.buffer() 
        << " AND r.handle = " << Database::Value(registrarHandle);
    Database::Connection conn = Database::Manager::acquire();
    Database::Result getCreditRes = conn.exec(getCreditQuery);
    if (getCreditRes.size() == 0) {
        LOGGER(PACKAGE).error("Cannot get registrar credit from database");
        return Database::Money();
    }
    Database::Money retval;
    retval = *(*getCreditRes.begin()).begin();
    return retval;
} // Manager::getCreditByZone()

void
Manager::archiveInvoices(bool send) const
{
    TRACE("[CALL] Register::Invoicing::Manager::archiveInvoices(bool)");
    try {
        // archive unarchived invoices
        ExporterArchiver exporter(m_docMan);
        List list((Manager *)this);
        Database::Filters::Union filter;
        Database::Filters::Invoice *invoice_filter = 
            new Database::Filters::InvoiceImpl();
        invoice_filter->addFilePDF().setNULL();
        filter.addFilter(invoice_filter);
        list.reload(filter);
        list.doExport(&exporter);
        if (send) {
            Mails mail(m_mailMan);
            mail.load();
            mail.send();
        }
    }
    catch (...) {
        LOGGER(PACKAGE).error("Register::Manager::archiveInvoices(bool) error");
    }
} // Manager::archiveInvoices()

Database::Money 
Manager::countVat(
        const Database::Money &price,
        const unsigned int &vatRate,
        const bool &base)
{
    TRACE("[CALL] Register::Invoicing::Manager::countVat("
            "Database::Money, unsigned int, bool)");
    unsigned int koef;
    Database::Query query;
    query.buffer()
        << "SELECT 10000*koef FROM price_vat "
        << "WHERE vat=" << Database::Value(vatRate);
    Database::Connection conn = Database::Manager::acquire();
    try {
        Database::Result res = conn.exec(query);
        koef = res[0][0];
    } catch (...) {
        LOGGER(PACKAGE).error("countVat: error");
        return Database::Money();
    }
    return price * koef / (10000 - (base ? koef : 0));
}

bool
Manager::insertInvoicePrefix(
        const std::string &zoneName,
        const int &type,
        const int &year,
        const unsigned long long &prefix)
{
    Database::Query query;
    query.buffer()
        << "SELECT id FROM zone WHERE fqdn=" << Database::Value(zoneName);
    try {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result res = conn.exec(query);
        if (res.size() == 0) {
            LOGGER(PACKAGE).error(boost::format("Unknown zone fqdn (%1%)")
                    % zoneName);
            return false;
        }
        unsigned long long zoneId = res[0][0];
        return insertInvoicePrefix(zoneId, type, year, prefix);
    } catch (...) {
        LOGGER(PACKAGE).error("An exception was catched.");
        return false;
    }
    return false;
}
bool 
Manager::insertInvoicePrefix(
        const unsigned long long &zoneId, 
        const int &type,
        const int &year,
        const unsigned long long &prefix) 
{
    TRACE("Invoicing::ManagerImpl::insertInvoicePrefix(...)");
    Database::InsertQuery insertPrefix("invoice_prefix");
    insertPrefix.add("zone", zoneId);
    insertPrefix.add("typ", type);
    insertPrefix.add("year", year);
    insertPrefix.add("prefix", prefix);
    Database::Connection conn = Database::Manager::acquire();
    try {
        conn.exec(insertPrefix);
    } catch (...) {
        LOGGER(PACKAGE).error("Failed to insert new invoice prefix");
        return false;
    }
    return true;
}

} // namespace Invoicing
} // namespace Register

