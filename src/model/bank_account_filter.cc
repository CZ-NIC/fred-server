#include "bank_account_filter.h"

namespace Database {
namespace Filters {

BankAccountImpl::BankAccountImpl(bool set_active)
{
    setName("BankAccount");
    active = set_active;
}

BankAccountImpl::~BankAccountImpl()
{
}

Table &
BankAccountImpl::joinBankAccountTable()
{
    return joinTable("bank_account");
}

Value<Database::ID> &
BankAccountImpl::addId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(
            Column("id", joinBankAccountTable()));
    add(tmp);
    tmp->setName("Id");
    return *tmp;
}

Value<std::string>& BankAccountImpl::addAccountNumber()
{
    Value<std::string> *tmp = new Value<std::string>(Column("account_number", joinBankAccountTable()));
    tmp->setName("AccountNumber");
    add(tmp);
    return *tmp;
}

Value<std::string>& BankAccountImpl::addAccountName()
{
    Value<std::string> *tmp = new Value<std::string>(Column("account_name", joinBankAccountTable()));
    tmp->setName("AccountName");
    add(tmp);
    return *tmp;
}

Value<std::string>& BankAccountImpl::addBankCode()
{
    Value<std::string> *tmp = new Value<std::string>(Column("bank_code", joinBankAccountTable()));
    tmp->setName("BankCode");
    add(tmp);
    return *tmp;
}

} // namespace Filters
} // namespace Database
