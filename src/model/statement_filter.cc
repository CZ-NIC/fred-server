#include "statement_filter.h"

namespace Database {
namespace Filters {

StatementImpl::StatementImpl():
    Compound()
{
    setName("Statement");
    active = true;
}

StatementImpl::~StatementImpl()
{
}

Table &
StatementImpl::joinStatementTable()
{
    return joinTable("bank_statement_head");
}

Table &
StatementImpl::joinStatementItemTable()
{
    return joinTable("bank_statement_item");
}

Value<Database::ID> &
StatementImpl::addId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(
            Column("id", joinStatementTable()));
    add(tmp);
    tmp->setName("Id");
    return *tmp;
}

Value<Database::ID> &
StatementImpl::addAccountId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(
            Column("account_id", joinStatementTable()));
    add(tmp);
    tmp->setName("AccountId");
    return *tmp;
}

Interval<Database::DateInterval> &
StatementImpl::addCreateDate()
{
    Interval<Database::DateInterval> *tmp =
        new Interval<Database::DateInterval>(
                Column("create_date", joinStatementTable()));
    add(tmp);
    tmp->setName("CreateDate");
    return *tmp;
}

Interval<Database::DateInterval> &
StatementImpl::addBalanceOldDate()
{
    Interval<Database::DateInterval> *tmp =
        new Interval<Database::DateInterval>(
                Column("balance_old_date", joinStatementTable()));
    add(tmp);
    tmp->setName("BalanceOldDate");
    return *tmp;
}

Value<std::string> &
StatementImpl::addAccountNumber()
{
    joins.push_back(new Join(
                Column("id", joinStatementTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<std::string> *tmp = new Value<std::string>(
            Column("account_number", joinStatementItemTable()));
    add(tmp);
    tmp->setName("AccountNumber");
    return *tmp;
}

Value<std::string> &
StatementImpl::addBankCode()
{
    joins.push_back(new Join(
                Column("id", joinStatementTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<std::string> *tmp = new Value<std::string>(
            Column("bank_code", joinStatementItemTable()));
    add(tmp);
    tmp->setName("BankCode");
    return *tmp;
}

Value<std::string> &
StatementImpl::addConstSymbol()
{
    joins.push_back(new Join(
                Column("id", joinStatementTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<std::string> *tmp = new Value<std::string>(
            Column("konstsym", joinStatementItemTable()));
    add(tmp);
    tmp->setName("ConstSymbol");
    return *tmp;
}

Value<std::string> &
StatementImpl::addVarSymbol()
{
    joins.push_back(new Join(
                Column("id", joinStatementTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<std::string> *tmp = new Value<std::string>(
            Column("varsymb", joinStatementItemTable()));
    add(tmp);
    tmp->setName("VarSymbol");
    return *tmp;
}

Value<std::string> &
StatementImpl::addSpecSymbol()
{
    joins.push_back(new Join(
                Column("id", joinStatementTable()),
                SQL_OP_EQ,
                Column("statement_id", joinStatementItemTable())));
    Value<std::string> *tmp = new Value<std::string>(
            Column("specsymb", joinStatementItemTable()));
    add(tmp);
    tmp->setName("SpecSymbol");
    return *tmp;
}

Value<Database::ID> &
StatementImpl::addInvoiceId()
{
    joins.push_back(new Join(
                Column("id", joinStatementTable()),
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

