#ifndef BANK_STATEMENT_IMPL_HH_1F29CB52BC574DEFBB8C89A84C799C60
#define BANK_STATEMENT_IMPL_HH_1F29CB52BC574DEFBB8C89A84C799C60

#include "src/libfred/banking/bank_common.hh"
#include "src/libfred/banking/bank_statement.hh"
#include "src/libfred/banking/model_bank_statement.hh"
#include "src/libfred/banking/bank_payment_impl.hh"
#include "src/libfred/common_impl_new.hh"
#include "src/libfred/db_settings.hh"
#include "src/util/types/date.hh"
#include "src/util/types/money.hh"


namespace LibFred {
namespace Banking {


class StatementImpl : virtual public Statement,
                      public LibFred::CommonObjectImplNew,
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
    Money getBalanceOld() const
    {
        return Money(ModelBankStatement::getBalanceOld());
    }
    Money getBalanceNew() const
    {
        return Money(ModelBankStatement::getBalanceNew());
    }
    Money getBalanceCredit() const
    {
        return Money(ModelBankStatement::getBalanceCredit());
    }
    Money getBalanceDebet() const
    {
        return Money(ModelBankStatement::getBalanceDebet());
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
    void setBalanceOld(const Money &balanceOld)
    {
        ModelBankStatement::setBalanceOld(balanceOld.get_string());
    }
    void setBalanceNew(const Money &balanceNew)
    {
        ModelBankStatement::setBalanceNew(balanceNew.get_string());
    }
    void setBalanceCredit(const Money &balanceCredit)
    {
        ModelBankStatement::setBalanceCredit(balanceCredit.get_string());
    }
    void setBalanceDebet(const Money &balanceDebet)
    {
        ModelBankStatement::setBalanceDebet(balanceDebet.get_string());
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

    void save()
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
        Money zero("0");
        try
        {
            bool crdate_is_special = getCreateDate().is_special();
            bool balanceolddate_is_special = getBalanceOldDate().is_special();
            bool balance_new_is_zero = (getBalanceNew() == zero);
            bool balance_old_is_zero = (getBalanceOld() == zero);
            bool balance_credit_is_zero = (getBalanceCredit() == zero);
            bool balance_debet_is_zero = (getBalanceDebet() == zero);

            if (getAccountId() != 0
                && crdate_is_special
                && balanceolddate_is_special
                && balance_new_is_zero
                && balance_old_is_zero
                && balance_credit_is_zero
                && balance_debet_is_zero) 
                {
                    return false;
                }
        }
        catch(const std::exception& ex)
        {
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

typedef std::unique_ptr<StatementImpl> StatementImplPtr;


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
    TRACE("[CALL] LibFred::Banking::statement_from_xml(...)");

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
        Money money(_node.getChild(STATEMENT_OLD_BALANCE).getValue());
        statement->setBalanceOld(money);
    }
    if (!_node.getChild(STATEMENT_OLD_BALANCE).isEmpty()) {
        Money money(_node.getChild(STATEMENT_BALANCE).getValue());
        statement->setBalanceNew(money);
    }
    if (!_node.getChild(STATEMENT_CREDIT).isEmpty()) {
        Money money(_node.getChild(STATEMENT_CREDIT).getValue());
        statement->setBalanceCredit(money);
    }
    if (!_node.getChild(STATEMENT_DEBET).isEmpty()) {
        Money money(_node.getChild(STATEMENT_DEBET).getValue());
        statement->setBalanceDebet(money);
    }

    return statement;
}

StatementImplPtr statement_from_params(
        const std::string& _account_number,
        const std::string& _account_bank_code)
{
    TRACE("[CALL] LibFred::Banking::statement_from_params(...)");

    if (_account_number.empty() || _account_bank_code.empty()) {
        throw std::runtime_error("not valid account number");
    }

    if (_account_number.empty() || _account_bank_code.empty()) {
        throw std::runtime_error(str(boost::format("could not get valid "
                    "account_number and account_bank_code (%1%/%2%)")
                    % _account_number % _account_bank_code));
    }

    Database::Query query;
    query.buffer() << "SELECT id FROM bank_account WHERE "
                   << "trim(leading '0' from account_number) = "
                   << "trim(leading '0' from " << Database::Value(_account_number) << ") "
                   << "AND bank_code = " << Database::Value(_account_bank_code);
    Database::Connection conn = Database::Manager::acquire();
    Database::Result result = conn.exec(query);
    if (result.size() == 0) {
        throw std::runtime_error(str(boost::format("not valid record found "
                    "in database for account=%1% bankcode=%2%")
                    % _account_number % _account_bank_code));
    }
    unsigned long long account_id = result[0][0];

    /* fill statement */
    StatementImplPtr statement(new StatementImpl());
    statement->setAccountId(account_id);

    return statement;
}


}
}


#endif /*BANK_STATEMENT_IMPL_H_*/

