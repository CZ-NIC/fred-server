#ifndef BANK_STATEMENT_IMPL_H_
#define BANK_STATEMENT_IMPL_H_

#include "bank_common.h"
#include "bank_statement.h"
#include "model_bank_statement.h"
#include "bank_payment_impl.h"
#include "common_impl_new.h"
#include "db_settings.h"


namespace Register {
namespace Banking {


class StatementImpl : virtual public Statement,
                      public Register::CommonObjectImplNew,
                      private ModelBankStatement
{
private:
    // TODO inner payments
    typedef std::vector<PaymentImpl> PaymentList;
    PaymentList payments_;


public:
    StatementImpl() : ModelBankStatement()
    {
    }

    const unsigned long long &getId() const
    {
        return ModelBankStatement::getId();
    }
    const unsigned long long &getAccountId() const
    {
        return ModelBankStatement::getAccountId();
    }
    const int &getNum() const
    {
        return ModelBankStatement::getNum();
    }
    const Database::Date &getCreateDate() const
    {
        return ModelBankStatement::getCreateDate();
    }
    const Database::Date &getBalanceOldDate() const
    {
        return ModelBankStatement::getBalanceOldDate();
    }
    const Database::Money &getBalanceOld() const
    {
        return ModelBankStatement::getBalanceOld();
    }
    const Database::Money &getBalanceNew() const
    {
        return ModelBankStatement::getBalanceNew();
    }
    const Database::Money &getBalanceCredit() const
    {
        return ModelBankStatement::getBalanceCredit();
    }
    const Database::Money &getBalanceDebet() const
    {
        return ModelBankStatement::getBalanceDebet();
    }
    const unsigned long long &getFileId() const
    {
        return ModelBankStatement::getFileId();
    }
    void setId(const unsigned long long &id)
    {
        ModelBankStatement::setId(id);
    }
    void setAccountId(const unsigned long long &accountId)
    {
        ModelBankStatement::setAccountId(accountId);
    }
    void setNum(const int &num)
    {
        ModelBankStatement::setNum(num);
    }
    void setCreateDate(const Database::Date &createDate)
    {
        ModelBankStatement::setCreateDate(createDate);
    }
    void setBalanceOldDate(const Database::Date &balanceOldDate)
    {
        ModelBankStatement::setBalanceOldDate(balanceOldDate);
    }
    void setBalanceOld(const Database::Money &balanceOld)
    {
        ModelBankStatement::setBalanceOld(balanceOld);
    }
    void setBalanceNew(const Database::Money &balanceNew)
    {
        ModelBankStatement::setBalanceNew(balanceNew);
    }
    void setBalanceCredit(const Database::Money &balanceCredit)
    {
        ModelBankStatement::setBalanceCredit(balanceCredit);
    }
    void setBalanceDebet(const Database::Money &balanceDebet)
    {
        ModelBankStatement::setBalanceDebet(balanceDebet);
    }
    void setFileId(const unsigned long long &fileId)
    {
        ModelBankStatement::setFileId(fileId);
    }

    // TODO
    unsigned int getPaymentCount() const
    {
        return 0;
    }
    Payment* getPaymentByIdx(const unsigned long long _id) const
    {
        return 0;
    }


    std::string toString() const
    {
        return ModelBankStatement::toString();
    }

    void save() throw (SQL_ERROR)
    {
        try {
            if (getId() == 0) {
                ModelBankStatement::insert();
            }
            else {
                ModelBankStatement::update();
            }
        }
        catch (std::exception &ex) {
            LOGGER(PACKAGE).error(
                    boost::format("Statement save: %1%") % ex.what());
            throw SQL_ERROR();
        }
        catch (...) {
            LOGGER(PACKAGE).error("Statement save: unknown exception");
            throw SQL_ERROR();
        }
    }

    bool isValid() const
    {
        Database::Money zero(0);
        if (getAccountId() != 0
                && getCreateDate().is_special()
                && getBalanceOldDate().is_special()
                && getBalanceNew() == zero
                && getBalanceOld() == zero
                && getBalanceCredit() == zero
                && getBalanceDebet() == zero) {

            return false;
        }
        return true;
    }

    Database::ID getConflictId()
    {
        Database::Query query;
        if (!isValid()) {
            query.buffer()
                << "SELECT id FROM bank_statement WHERE num ISNULL"
                << " AND account_id= " << Database::Value(getAccountId())
                << " AND create_date ISNULL";
        } else {
            query.buffer()
                << "SELECT id FROM bank_statement"
                << " WHERE num = " << Database::Value(getNum())
                << " AND account_id = " << Database::Value(getAccountId())
                << " AND create_date = " << Database::Value(getCreateDate());
        }
        Database::Connection conn = Database::Manager::acquire();
        Database::Result result = conn.exec(query);
        if (result.size() == 1) {
            return result[0][0];

        }
        return Database::ID(0);
    }
};

typedef std::auto_ptr<StatementImpl> StatementImplPtr;


COMPARE_CLASS_IMPL_NEW(StatementImpl, AccountId);
COMPARE_CLASS_IMPL_NEW(StatementImpl, Num);
COMPARE_CLASS_IMPL_NEW(StatementImpl, CreateDate);
COMPARE_CLASS_IMPL_NEW(StatementImpl, BalanceOldDate);
COMPARE_CLASS_IMPL_NEW(StatementImpl, BalanceOld);
COMPARE_CLASS_IMPL_NEW(StatementImpl, BalanceNew);
COMPARE_CLASS_IMPL_NEW(StatementImpl, BalanceCredit);
COMPARE_CLASS_IMPL_NEW(StatementImpl, BalanceDebet);
COMPARE_CLASS_IMPL_NEW(StatementImpl, FileId);


StatementImplPtr parse_xml_statement_part(const XMLnode &_node)
{
    TRACE("[CALL] Register::Banking::statement_from_xml(...)");

    /* manual xml validation */
    if (!_node.hasChild(STATEMENT_ACCOUNT_NUMBER)
            || !_node.hasChild(STATEMENT_ACCOUNT_NUMBER)
            || !_node.hasChild(STATEMENT_DATE)
            || !_node.hasChild(STATEMENT_OLD_DATE)
            || !_node.hasChild(STATEMENT_BALANCE)
            || !_node.hasChild(STATEMENT_OLD_BALANCE)
            || !_node.hasChild(STATEMENT_CREDIT)
            || !_node.hasChild(STATEMENT_DEBET)) {

        throw std::runtime_error("not valid statement xml");
    }

    /* get bank account id */
    std::string account_number = _node.getChild(STATEMENT_ACCOUNT_NUMBER).getValue();
    std::string account_bank_code = _node.getChild(STATEMENT_ACCOUNT_BANK_CODE).getValue();

    if (account_number.empty() || account_bank_code.empty()) {
        throw std::runtime_error(str(boost::format("could not get valid "
                    "account_number and account_bank_code (%1%/%2%)")
                    % account_number % account_bank_code));
    }

    Database::Query query;
    query.buffer() << "SELECT id FROM bank_account WHERE "
                   << "trim(leading '0' from account_number) = "
                   << "trim(leading '0' from " << Database::Value(account_number) << ") "
                   << "AND bank_code = " << Database::Value(account_bank_code);
    Database::Connection conn = Database::Manager::acquire();
    Database::Result result = conn.exec(query);
    if (result.size() == 0) {
        throw std::runtime_error(str(boost::format("not valid record found "
                    "in database for account=%1% bankcode=%2%")
                    % account_number % account_bank_code));
    }
    unsigned long long account_id = result[0][0];

    /* fill statement */
    StatementImplPtr statement(new StatementImpl());
    statement->setAccountId(account_id);
    if (!_node.getChild(STATEMENT_NUMBER).isEmpty()) {
        statement->setNum(atoi(_node.getChild(STATEMENT_NUMBER).getValue().c_str()));
    }
    if (!_node.getChild(STATEMENT_DATE).isEmpty()) {
        statement->setCreateDate(Database::Date(_node.getChild(STATEMENT_DATE).getValue()));
    }
    if (!_node.getChild(STATEMENT_OLD_DATE).isEmpty()) {
        statement->setBalanceOldDate(Database::Date(_node.getChild(STATEMENT_OLD_DATE).getValue()));
    }
    if (!_node.getChild(STATEMENT_BALANCE).isEmpty()) {
        Database::Money money;
        money.format(_node.getChild(STATEMENT_OLD_BALANCE).getValue());
        statement->setBalanceOld(money);
    }
    if (!_node.getChild(STATEMENT_OLD_BALANCE).isEmpty()) {
        Database::Money money;
        money.format(_node.getChild(STATEMENT_BALANCE).getValue());
        statement->setBalanceNew(money);
    }
    if (!_node.getChild(STATEMENT_CREDIT).isEmpty()) {
        Database::Money money;
        money.format(_node.getChild(STATEMENT_CREDIT).getValue());
        statement->setBalanceCredit(money);
    }
    if (!_node.getChild(STATEMENT_DEBET).isEmpty()) {
        Database::Money money;
        money.format(_node.getChild(STATEMENT_DEBET).getValue());
        statement->setBalanceDebet(money);
    }

    return statement;
}


}
}


#endif /*BANK_STATEMENT_IMPL_H_*/

