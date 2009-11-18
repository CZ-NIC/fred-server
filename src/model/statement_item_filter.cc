#include "statement_item_filter.h"

namespace Database {
namespace Filters {

StatementItemImpl::StatementItemImpl():
    Compound()
{
    setName("StatementItem");
    active = true;
}

StatementItemImpl::~StatementItemImpl()
{
}

Table &
StatementItemImpl::joinStatementItemTable()
{
    return joinTable("bank_item");
}

Value<Database::ID> &
StatementItemImpl::addId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(
            Column("id", joinStatementItemTable()));
    add(tmp);
    tmp->setName("Id");
    return *tmp;
}

Value<std::string> &
StatementItemImpl::addAccountNumber()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("account_number", joinStatementItemTable()));
    add(tmp);
    tmp->setName("AccountNumber");
    return *tmp;
}

Value<std::string> &
StatementItemImpl::addBankCode()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("bank_code", joinStatementItemTable()));
    add(tmp);
    tmp->setName("BankCode");
    return *tmp;
}

Value<int> &
StatementItemImpl::addCode()
{
    Value<int> *tmp = new Value<int>(
            Column("code", joinStatementItemTable()));
    add(tmp);
    tmp->setName("Code");
    return *tmp;
}

Value<int> &
StatementItemImpl::addType()
{
    Value<int> *tmp = new Value<int>(
            Column("code", joinStatementItemTable()));
    add(tmp);
    tmp->setName("Code");
    return *tmp;
}

Value<std::string> &
StatementItemImpl::addConstSymb()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("konstsym", joinStatementItemTable()));
    add(tmp);
    tmp->setName("ConstSymb");
    return *tmp;
}

Value<std::string> &
StatementItemImpl::addVarSymb()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("varsymb", joinStatementItemTable()));
    add(tmp);
    tmp->setName("VarSymb");
    return *tmp;
}

Value<std::string> &
StatementItemImpl::addSpecSymb()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("specsymb", joinStatementItemTable()));
    add(tmp);
    tmp->setName("SpecSymb");
    return *tmp;
}

Value<std::string> &
StatementItemImpl::addAccountEvid()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("account_evid", joinStatementItemTable()));
    add(tmp);
    tmp->setName("AccountEvid");
    return *tmp;
}

Interval<Database::DateInterval> &
StatementItemImpl::addAccountDate()
{
    Interval<Database::DateInterval> *tmp =
        new Interval<Database::DateInterval>(
            Column("account_date", joinStatementItemTable()));
    add(tmp);
    tmp->setName("AccountDate");
    return *tmp;
}

Value<Database::ID> &
StatementItemImpl::addInvoiceId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(
            Column("invoice_id", joinStatementItemTable()));
    add(tmp);
    tmp->setName("InvoiceId");
    return *tmp;
}

Value<std::string> &
StatementItemImpl::addAccountName()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("account_name", joinStatementItemTable()));
    add(tmp);
    tmp->setName("AccountName");
    return *tmp;
}

Interval<Database::DateTimeInterval> &
StatementItemImpl::addCrTime()
{
    Interval<Database::DateTimeInterval> *tmp = 
        new Interval<Database::DateTimeInterval>(
            Column("crtime", joinStatementItemTable()));
    add(tmp);
    tmp->setName("CrTime");
    return *tmp;
}

StatementHead &
StatementItemImpl::addStatementHead()
{
    StatementHead *tmp = new StatementHeadImpl();
    tmp->joinOn(new Join(
                Column("statement_id", joinStatementItemTable()),
                SQL_OP_EQ,
                Column("id", tmp->joinStatementHeadTable())
                ));
    add(tmp);
    tmp->setName("StatementHead");
    return *tmp;
}

} // namespace Filters
} // namespace Database
