#include "statement_filter.h"

namespace Database {
namespace Filters {

StatementHeadImpl::StatementHeadImpl():
    Compound()
{
    setName("Statement");
    active = true;
}

StatementHeadImpl::~StatementHeadImpl()
{
}

Table &
StatementHeadImpl::joinStatementHeadTable()
{
    return joinTable("bank_head");
}

Table &
StatementHeadImpl::joinStatementItemTable()
{
    return joinTable("bank_item");
}

Value<Database::ID> &
StatementHeadImpl::addId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(
            Column("id", joinStatementHeadTable()));
    add(tmp);
    tmp->setName("Id");
    return *tmp;
}

Value<Database::ID> &
StatementHeadImpl::addAccountId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(
            Column("account_id", joinStatementHeadTable()));
    add(tmp);
    tmp->setName("AccountId");
    return *tmp;
}

Interval<Database::DateInterval> &
StatementHeadImpl::addCreateDate()
{
    Interval<Database::DateInterval> *tmp =
        new Interval<Database::DateInterval>(
                Column("create_date", joinStatementHeadTable()));
    add(tmp);
    tmp->setName("CreateDate");
    return *tmp;
}

Interval<Database::DateInterval> &
StatementHeadImpl::addBalanceOldDate()
{
    Interval<Database::DateInterval> *tmp =
        new Interval<Database::DateInterval>(
                Column("balance_old_date", joinStatementHeadTable()));
    add(tmp);
    tmp->setName("BalanceOldDate");
    return *tmp;
}

Value<std::string> &
StatementHeadImpl::addAccountNumber()
{
    joins.push_back(new Join(
                Column("id", joinStatementHeadTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<std::string> *tmp = new Value<std::string>(
            Column("account_number", joinStatementItemTable()));
    add(tmp);
    tmp->setName("AccountNumber");
    return *tmp;
}

Value<std::string> &
StatementHeadImpl::addBankCode()
{
    joins.push_back(new Join(
                Column("id", joinStatementHeadTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<std::string> *tmp = new Value<std::string>(
            Column("bank_code", joinStatementItemTable()));
    add(tmp);
    tmp->setName("BankCode");
    return *tmp;
}

Value<std::string> &
StatementHeadImpl::addConstSymbol()
{
    joins.push_back(new Join(
                Column("id", joinStatementHeadTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<std::string> *tmp = new Value<std::string>(
            Column("konstsym", joinStatementItemTable()));
    add(tmp);
    tmp->setName("ConstSymbol");
    return *tmp;
}

Value<std::string> &
StatementHeadImpl::addVarSymbol()
{
    joins.push_back(new Join(
                Column("id", joinStatementHeadTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<std::string> *tmp = new Value<std::string>(
            Column("varsymb", joinStatementItemTable()));
    add(tmp);
    tmp->setName("VarSymbol");
    return *tmp;
}

Value<std::string> &
StatementHeadImpl::addSpecSymbol()
{
    joins.push_back(new Join(
                Column("id", joinStatementHeadTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<std::string> *tmp = new Value<std::string>(
            Column("specsymb", joinStatementItemTable()));
    add(tmp);
    tmp->setName("SpecSymbol");
    return *tmp;
}

Value<Database::ID> &
StatementHeadImpl::addInvoiceId()
{
    joins.push_back(new Join(
                Column("id", joinStatementHeadTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<Database::ID> *tmp = new Value<Database::ID>(
            Column("invoice_id", joinStatementItemTable()));
    add(tmp);
    tmp->setName("InvoiceId");
    return *tmp;
}


} // namespace Database
} // namespace Filters

