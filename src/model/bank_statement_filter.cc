#include "bank_statement_filter.h"

namespace Database {
namespace Filters {

BankStatementImpl::BankStatementImpl():
    Compound()
{
    setName("Statement");
    active = true;
}

BankStatementImpl::~BankStatementImpl()
{
}

Table &
BankStatementImpl::joinBankStatementTable()
{
    return joinTable("bank_statement");
}

Table &
BankStatementImpl::joinStatementItemTable()
{
    return joinTable("bank_payment");
}

Value<Database::ID> &
BankStatementImpl::addId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(
            Column("id", joinBankStatementTable()));
    add(tmp);
    tmp->setName("Id");
    return *tmp;
}

Value<Database::ID> &
BankStatementImpl::addAccountId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(
            Column("account_id", joinBankStatementTable()));
    add(tmp);
    tmp->setName("AccountId");
    return *tmp;
}

Interval<Database::DateInterval> &
BankStatementImpl::addCreateDate()
{
    Interval<Database::DateInterval> *tmp =
        new Interval<Database::DateInterval>(
                Column("create_date", joinBankStatementTable()));
    add(tmp);
    tmp->setName("CreateDate");
    return *tmp;
}

Interval<Database::DateInterval> &
BankStatementImpl::addBalanceOldDate()
{
    Interval<Database::DateInterval> *tmp =
        new Interval<Database::DateInterval>(
                Column("balance_old_date", joinBankStatementTable()));
    add(tmp);
    tmp->setName("BalanceOldDate");
    return *tmp;
}

Value<std::string> &
BankStatementImpl::addAccountNumber()
{
    joins.push_back(new Join(
                Column("id", joinBankStatementTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<std::string> *tmp = new Value<std::string>(
            Column("account_number", joinStatementItemTable()));
    add(tmp);
    tmp->setName("AccountNumber");
    return *tmp;
}

Value<std::string> &
BankStatementImpl::addBankCode()
{
    joins.push_back(new Join(
                Column("id", joinBankStatementTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<std::string> *tmp = new Value<std::string>(
            Column("bank_code", joinStatementItemTable()));
    add(tmp);
    tmp->setName("BankCode");
    return *tmp;
}

Value<std::string> &
BankStatementImpl::addConstSymbol()
{
    joins.push_back(new Join(
                Column("id", joinBankStatementTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<std::string> *tmp = new Value<std::string>(
            Column("konstsym", joinStatementItemTable()));
    add(tmp);
    tmp->setName("ConstSymbol");
    return *tmp;
}

Value<std::string> &
BankStatementImpl::addVarSymbol()
{
    joins.push_back(new Join(
                Column("id", joinBankStatementTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<std::string> *tmp = new Value<std::string>(
            Column("varsymb", joinStatementItemTable()));
    add(tmp);
    tmp->setName("VarSymbol");
    return *tmp;
}

Value<std::string> &
BankStatementImpl::addSpecSymbol()
{
    joins.push_back(new Join(
                Column("id", joinBankStatementTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<std::string> *tmp = new Value<std::string>(
            Column("specsymb", joinStatementItemTable()));
    add(tmp);
    tmp->setName("SpecSymbol");
    return *tmp;
}


} // namespace Database
} // namespace Filters

