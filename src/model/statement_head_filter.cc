#include "statement_head_filter.h"

namespace Database {
namespace Filters {

StatementHeadImpl::StatementHeadImpl():
    Compound()
{
    setName("StatementItem");
    active = true;
}

StatementHeadImpl::~StatementHeadImpl()
{
}

Table &
StatementHeadImpl::joinStatementHeadTable()
{
    return joinTable("bank_statement_head");
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

Value<int> &
StatementHeadImpl::addNumber()
{
    Value<int> *tmp = new Value<int>(
            Column("num", joinStatementHeadTable()));
    add(tmp);
    tmp->setName("Number");
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

} // namespace Filters
} // namespace Database
