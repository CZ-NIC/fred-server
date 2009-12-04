#include "bank_head.h"

namespace Register {
namespace Banking {

void
StatementHead::doExport(Exporter *exp)
{
    TRACE("[CALL] Register::Banking::StatementHead::doExport(Exporter *exp)");
    exp->doExport(this);
}

unsigned int 
StatementHead::getStatementItemCount() const 
{
    return m_statementItems.size();
}
const StatementItem *
StatementHead::getStatementItemByIdx(const unsigned int &idx) const
{
    if (idx >= getStatementItemCount()) {
       return NULL;
    }
    return &m_statementItems[idx];
}
StatementItem *
StatementHead::newStatementItem()
{
    StatementItem item;
    m_statementItems.push_back(item);
    return &m_statementItems.at(m_statementItems.size() - 1);
}
StatementItem *
StatementHead::addStatementItem(const StatementItem &statementItem)
{
    m_statementItems.push_back(statementItem);
    return &m_statementItems.at(m_statementItems.size() - 1);
}

/* Try to guess if header is from ebanka csv file - such header
 * has only proper account number. Money amounts are all equal
 * to zero and dates are invalid.
 * Return value true means that this is csv header
 */
bool 
StatementHead::isCsvHeader()
{
    TRACE("[CALL] Register::Banking::StatementHead::guessCsvHeader()");
    if (getNum() != 0) {
        return false;
    }
    if (getCreateDate() != Database::Date()) {
        return false;
    }
    if (getBalanceOldDate() != Database::Date()) {
        return false;
    }
    Database::Money zeroValue(0);
    if (getBalanceNew() != zeroValue) {
        return false;
    }
    if (getBalanceOld() != zeroValue) {
        return false;
    }
    if (getBalanceCredit() != zeroValue) {
        return false;
    }
    if (getBalanceDebet() != zeroValue) {
        return false;
    }
    return true;
}

Database::ID
StatementHead::getConflictId()
{
    TRACE("[CALL] Register::Banking::StatementHead::getConflictId()");
    Database::Query query;
    if (isCsvHeader()) {
        query.buffer()
            << "SELECT id FROM bank_head WHERE num ISNULL "
            << "AND account_id= " << Database::Value(getAccountId())
            << " AND create_date ISNULL";
    } else {
        query.buffer()
            << "SELECT id FROM bank_head "
            << "WHERE num = " << Database::Value(getNum())
            << " AND account_id = " << Database::Value(getAccountId())
            << " AND create_date = " << Database::Value(getCreateDate());
    }
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec(query);
    if (res.size() == 0) {
        return Database::ID();
    }
    Database::ID ret = res[0][0];
    return ret;
}

bool 
StatementHead::isExisting()
{
    TRACE("[CALL] Register::Banking::StatementHead::isExisting()");
    if (getConflictId() == Database::ID()) {
        LOGGER(PACKAGE).debug("header not exists in the database");
        return false;
    }
    LOGGER(PACKAGE).warning("payment header duplicity found");
    return true;
}

/* update columns in bank_account table: balance, last_date and
 * last_num
 */
bool 
StatementHead::updateBankAccount()
{
    TRACE("[CALL] Register::Banking::StatementHead::updateBankAccount()");
    Database::Query query;
    query.buffer()
        << "UPDATE bank_account SET balance = balance + " <<
        ((getBalanceCredit() == Database::Money()) ? Database::Value(getBalanceDebet()) : Database::Value(getBalanceCredit()))
        << ", last_date = " << Database::Value(getCreateDate()) 
        << ", last_num = " << Database::Value(getNum())
        << " WHERE id = " << Database::Value(getAccountId());
    Database::Connection conn = Database::Manager::acquire();
    try {
        conn.exec(query);
    } catch (...) {
        LOGGER(PACKAGE).error("Unable to update ``bank_account'' table");
        return false;
    }
    return true;
}

bool 
StatementHead::save()
{
    TRACE("[CALL] Register::Banking::StatementHead::save()");
    try {
        if (getId() == Database::ID()) {
            insert();
        } else {
            update();
        }
    } catch (...) {
        return false;
    }
    return true;
}
StatementItem *
StatementHead::createStatementItem()
{
    return addStatementItem(StatementItem());
}
Database::ID 
StatementHead::getBankNumberId(
        const std::string &account_num, 
        const std::string &bank_code)
{
    Database::Query query;
    query.buffer()
        << "select id from bank_account where " 
        << "trim(leading '0' from account_number) = "
        << "trim(leading '0' from "
        << Database::Value(account_num) << ") and bank_code = "
        << Database::Value(bank_code);
    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec(query);
    if (res.size() == 0) {
        return Database::ID();
    }
    return *(*res.begin()).begin();
}
Database::ID 
StatementHead::getBankNumberId(
        const std::string &account_num)
{
    std::string account;
    std::string code;
    account = account_num.substr(0, account_num.find('/'));
    code = account_num.substr(account_num.find('/') + 1, std::string::npos);
    return getBankNumberId(account, code);
}

bool
StatementHead::deleteHead()
{
    Database::Query query;
    query.buffer() << "DELETE FROM bank_head WHERE id="
        << Database::Value(getId());
    Database::Connection conn = Database::Manager::acquire();
    try {
        conn.exec(query);
    } catch (...) {
        LOGGER(PACKAGE).error(
                boost::format("Failed to delete bank statement head (id: %1%)"
                    " from database") % getId());
        return false;
    }
    return true;
}

bool 
StatementHead::fromXML(
        const XMLnode &node, 
        const unsigned long long &fileId)
{
    TRACE("[CALL] Register::Banking::Statement::fromXML()");
    TEST_NODE_PRESENCE(node, STATEMENT_ACCOUNT_NUMBER);
    setAccountId(getBankNumberId(
                node.getChild(STATEMENT_ACCOUNT_NUMBER).getValue()));
    if (getAccountId() == Database::ID()) {
        LOGGER(PACKAGE).error("account number does not exist in db");
        return false;
    }
    if (fileId != 0) {
        setFileId(fileId);
    }
    if (!node.getChild(STATEMENT_NUMBER).isEmpty()) {
        setNum(atoi(node.getChild(STATEMENT_NUMBER).getValue().c_str()));
    }
    if (!node.getChild(STATEMENT_DATE).isEmpty()) {
        setCreateDate(Database::Date(node.getChild(STATEMENT_DATE).getValue()));
    }
    if (!node.getChild(STATEMENT_OLD_DATE).isEmpty()) {
        setBalanceOldDate(Database::Date(node.getChild(STATEMENT_OLD_DATE).getValue()));
    }
    Database::Money money;
    if (!node.getChild(STATEMENT_BALANCE).isEmpty()) {
        money.format(node.getChild(STATEMENT_OLD_BALANCE).getValue());
        setBalanceOld(money);
    }
    if (!node.getChild(STATEMENT_OLD_BALANCE).isEmpty()) {
        money.format(node.getChild(STATEMENT_BALANCE).getValue());
        setBalanceNew(money);
    }
    if (!node.getChild(STATEMENT_CREDIT).isEmpty()) {
        money.format(node.getChild(STATEMENT_CREDIT).getValue());
        setBalanceCredit(money);
    }
    if (!node.getChild(STATEMENT_DEBET).isEmpty()) {
        money.format(node.getChild(STATEMENT_DEBET).getValue());
        setBalanceDebet(money);
    }
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction trans(conn);
    bool head_saved = false;
    bool item_saved = false;

    if (isCsvHeader()) {
        setId(0);
    } else {
        if (isExisting()) {
            setId(getConflictId());
        } else {
            if (!save()) {
                LOGGER(PACKAGE).error("Failed to insert Statement head");
                return false;
            }
            head_saved = true;
        }
    }
    
    XMLnode items = node.getChild(STATEMENT_ITEMS);
    for (int i = 0; i < items.getChildrenSize(); i++) {
        StatementItem *item = createStatementItem();
        if (getId() != Database::ID(0)) {
            item->setStatementId(getId());
        }
        Database::Transaction trans_item(conn);
        if (!item->fromXML(items.getChild(i))) {
            continue;
        }
        trans_item.commit();
        item_saved = true;
    }

    if (!head_saved && !item_saved) {
        LOGGER(PACKAGE).warning(
                "this payment is probably existing in the database "
                "(neither head nor any item saved)");
    } else if (head_saved && !item_saved && isCsvHeader()) {
        LOGGER(PACKAGE).info("CSV (empty) head without item won't be saved");
        return true;
    } else if (head_saved && !item_saved) {
        LOGGER(PACKAGE).info("Head without any item saved");
    }
    trans.commit();
    return true;
} // StatementHead::fromXML(XMLnode)

/*
const std::string &
StatementHead::getZoneFqdn()
{
    return getAccount()->getZone()->getFqdn();
}

const std::string &
StatementHead::getAccountNumber()
{
    return getAccount()->getAccountNumber();
}
const std::string &
StatementHead::getBankCode()
{
    return getAccount()->getBankCode();
}
*/
} // namespace Banking
} // namespace Register
