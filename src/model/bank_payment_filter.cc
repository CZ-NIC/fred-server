#include "bank_payment_filter.h"

namespace Database {
namespace Filters {

BankPaymentImpl::BankPaymentImpl():
    Compound()
{
    setName("BankPayment");
    active = true;
}

BankPaymentImpl::~BankPaymentImpl()
{
}

Table &
BankPaymentImpl::joinBankPaymentTable()
{
    return joinTable("bank_payment");
}

Value<Database::ID> &
BankPaymentImpl::addId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(
            Column("id", joinBankPaymentTable()));
    add(tmp);
    tmp->setName("Id");
    return *tmp;
}

Value<std::string> &
BankPaymentImpl::addAccountNumber()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("account_number", joinBankPaymentTable()));
    add(tmp);
    tmp->setName("AccountNumber");
    return *tmp;
}

Value<std::string> &
BankPaymentImpl::addBankCode()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("bank_code", joinBankPaymentTable()));
    add(tmp);
    tmp->setName("BankCode");
    return *tmp;
}

Value<int> &
BankPaymentImpl::addCode()
{
    Value<int> *tmp = new Value<int>(
            Column("code", joinBankPaymentTable()));
    add(tmp);
    tmp->setName("Code");
    return *tmp;
}

Value<int> &
BankPaymentImpl::addType()
{
    Value<int> *tmp = new Value<int>(
            Column("type", joinBankPaymentTable()));
    add(tmp);
    tmp->setName("Type");
    return *tmp;
}

Value<std::string> &
BankPaymentImpl::addConstSymb()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("konstsym", joinBankPaymentTable()));
    add(tmp);
    tmp->setName("ConstSymb");
    return *tmp;
}

Value<std::string> &
BankPaymentImpl::addVarSymb()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("varsymb", joinBankPaymentTable()));
    add(tmp);
    tmp->setName("VarSymb");
    return *tmp;
}

Value<std::string> &
BankPaymentImpl::addSpecSymb()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("specsymb", joinBankPaymentTable()));
    add(tmp);
    tmp->setName("SpecSymb");
    return *tmp;
}

Value<std::string> &
BankPaymentImpl::addAccountEvid()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("account_evid", joinBankPaymentTable()));
    add(tmp);
    tmp->setName("AccountEvid");
    return *tmp;
}

Interval<Database::DateInterval> &
BankPaymentImpl::addAccountDate()
{
    Interval<Database::DateInterval> *tmp =
        new Interval<Database::DateInterval>(
            Column("account_date", joinBankPaymentTable()));
    add(tmp);
    tmp->setName("AccountDate");
    return *tmp;
}

Value<std::string> &
BankPaymentImpl::addAccountName()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("account_name", joinBankPaymentTable()));
    add(tmp);
    tmp->setName("AccountName");
    return *tmp;
}

Interval<Database::DateTimeInterval> &
BankPaymentImpl::addCrTime()
{
    Interval<Database::DateTimeInterval> *tmp = 
        new Interval<Database::DateTimeInterval>(
            Column("crtime", joinBankPaymentTable()));
    add(tmp);
    tmp->setName("CrTime");
    return *tmp;
}


BankStatement &
BankPaymentImpl::addBankStatement()
{
    BankStatement *tmp = new BankStatementImpl();
    tmp->joinOn(new Join(
                Column("statement_id", joinBankPaymentTable()),
                SQL_OP_EQ,
                Column("id", tmp->joinBankStatementTable())
                ));
    add(tmp);
    tmp->setName("BankStatement");
    return *tmp;
}

Value<std::string> &
BankPaymentImpl::addAccountMemo()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("account_memo", joinBankPaymentTable()));
    add(tmp);
    tmp->setName("AccountMemo");
    return *tmp;
}

Value<Database::ID> &
BankPaymentImpl::addAccountId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(
            Column("account_id", joinBankPaymentTable()));
    add(tmp);
    tmp->setName("AccountId");
    return *tmp;
}

} // namespace Filters
} // namespace Database
