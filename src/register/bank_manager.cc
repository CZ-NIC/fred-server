#include <string>
#include <iostream>

#include "bank_manager.h"
#include "bank_common.h"
#include "invoice_manager.h"

namespace Register {
namespace Banking {

HeadList *
Manager::createList() const
{
    TRACE("[CALL] Register::Banking::Manager::createList()");
    return new HeadList();
}

ItemList *
Manager::createItemList() const
{
    TRACE("[CALL] Register::Banking::Manager::createItemList()");
    return new ItemList();
}

Manager *
Manager::create()
{
    TRACE("[CALL] Register::Banking::Manager::create()");
    return new Manager();
}

StatementHead *
Manager::createStatement() const
{
    TRACE("[CALL] Register::Banking::Manager::createStatement()");
    return new StatementHead();
}

bool 
Manager::importStatementXml(
        std::istream &in,
        const unsigned long long &fileId,
        const bool &createCreditInvoice)
{
    TRACE("[CALL] Register::Banking::Manager::importStatementXml(...)");
    std::string xml = loadInStream(in);
    XMLparser parser;
    if (!parser.parse(xml)) {
        return false;
    }
    XMLnode root = parser.getRootNode();
    if (root.getName().compare(STATEMENTS_ROOT) != 0) {
        LOGGER(PACKAGE).error(boost::format(
                    "XML: root element name is not ``%1%''")
                % STATEMENTS_ROOT);
        return false;
    }
    for (int i = 0; i < root.getChildrenSize(); i++) {
        XMLnode statement = root.getChild(i);
        std::auto_ptr<StatementHead> stat(createStatement());
        if (!stat->fromXML(statement, fileId)) {
            return false;
        }
    }
    if (createCreditInvoice) {
        std::auto_ptr<Invoicing::Manager>
            invMan(Invoicing::Manager::create());
        return invMan->pairInvoices();
    }
    return true;
}

bool
Manager::insertBankAccount(
        const std::string &zone,
        const std::string &account_number,
        const std::string &account_name,
        const std::string &bank_code)
{
    TRACE("[CALL] Register::Banking::Manager::insertBankAccount("
            "const std::string &, const std::string &, "
            "const std::string &, const std::string &)");
    Database::Query query;
    query.buffer()
        << "SELECT id FROM zone WHERE fqdn=" << Database::Value(zone);
    try {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result res = conn.exec(query);
        if (res.size() == 0) {
            LOGGER(PACKAGE).error(boost::format("Unknown zone (%1%)") % zone);
            return false;
        }
        Database::ID zoneId = res[0][0];
        return insertBankAccount(zoneId, account_number, account_name, bank_code);
    } catch (...) {
        LOGGER(PACKAGE).error("An exception was catched");
        return false;
    }
    return false;
}

bool 
Manager::insertBankAccount(
        const Database::ID &zone, 
        const std::string &account_number,
        const std::string &account_name,
        const std::string &bank_code)
{
    TRACE("[CALL] Register::Banking::Manager::insertBankAccount(...)");
    Database::InsertQuery insertAccount("bank_account");
    insertAccount.add("zone", zone);
    insertAccount.add("account_number", account_number);
    insertAccount.add("account_name", account_name);
    insertAccount.add("bank_code", bank_code);
    Database::Connection conn = Database::Manager::acquire();
    try {
        conn.exec(insertAccount);
    } catch (...) {
        LOGGER(PACKAGE).error("Cannot insert new account into the ``bank_account'' table");
        return false;
    }
    return true;
}

bool
Manager::moveItemToPayment(
        const Database::ID &paymentItem,
        const Database::ID &paymentHead,
        bool force)
{
    TRACE("[CALL] Register::Banking::Manager::moveItemToPayment()");
    Database::Query headQuery;
    headQuery.buffer()
        << "SELECT id FROM bank_head WHERE id="
        << Database::Value(paymentHead);
    Database::Query itemQuery;
    itemQuery.buffer()
        << "SELECT statement_id FROM bank_item WHERE id="
        << Database::Value(paymentItem);
    Database::Query moveQuery;
    moveQuery.buffer()
        << "UPDATE bank_item SET statement_id=";
    if (paymentHead == 0) {
        moveQuery.buffer() << "NULL";
    } else {
        moveQuery.buffer() << Database::Value(paymentHead);
    }
    moveQuery.buffer()
        << " WHERE id="
        << Database::Value(paymentItem);
    Database::Connection conn = Database::Manager::acquire();
    try {
        Database::Result headRes = conn.exec(headQuery);
        Database::Result itemRes = conn.exec(itemQuery);
        Database::ID statementId = itemRes[0][0];
        if (statementId != Database::ID() && !force) {
            LOGGER(PACKAGE).error(
                    "moveItemToPayment: trying to move payment item with "
                    "head without force");
            return false;
        }
        conn.exec(moveQuery);
    } catch (...) {
        LOGGER(PACKAGE).error("moveItemToPayment: an error has occured");
        return false;
    }
    return true;
}

} // namespace Banking
} // namespace Register

