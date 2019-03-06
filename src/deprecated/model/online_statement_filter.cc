/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/deprecated/model/online_statement_filter.hh"

namespace Database {
namespace Filters {

OnlineStatementImpl::OnlineStatementImpl():
    Compound()
{
    setName("OnlineStatement");
    active = true;
}

OnlineStatementImpl::~OnlineStatementImpl()
{
}

Table &
OnlineStatementImpl::joinOnlineStatementTable()
{
    return joinTable("bank_ebanka_list");
}

Value<Database::ID> &
OnlineStatementImpl::addId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(
            Column("id", joinOnlineStatementTable()));
    add(tmp);
    tmp->setName("Id");
    return *tmp;
}

Value<Database::ID> &
OnlineStatementImpl::addAccountId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(
            Column("account_id", joinOnlineStatementTable()));
    add(tmp);
    tmp->setName("AccountId");
    return *tmp;
}

Interval<Database::DateTimeInterval> &
OnlineStatementImpl::addCreateTime()
{
    Interval<Database::DateTimeInterval> *tmp =
        new Interval<Database::DateTimeInterval>(
                Column("create_date", joinOnlineStatementTable()));
    add(tmp);
    tmp->setName("CreateTime");
    return *tmp;
}

Value<std::string> &
OnlineStatementImpl::addAccountNumber()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("account_number", joinOnlineStatementTable()));
    add(tmp);
    tmp->setName("AccountNumber");
    return *tmp;
}

Value<std::string> &
OnlineStatementImpl::addBankCode()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("bank_code", joinOnlineStatementTable()));
    add(tmp);
    tmp->setName("BankCode");
    return *tmp;
}

Value<std::string> &
OnlineStatementImpl::addConstSymbol()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("konstsym", joinOnlineStatementTable()));
    add(tmp);
    tmp->setName("ConstSymbol");
    return *tmp;
}

Value<std::string> &
OnlineStatementImpl::addVarSymbol()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("varsymb", joinOnlineStatementTable()));
    add(tmp);
    tmp->setName("VarSymbol");
    return *tmp;
}

Value<std::string> &
OnlineStatementImpl::addAccountName()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("name", joinOnlineStatementTable()));
    add(tmp);
    tmp->setName("AccountName");
    return *tmp;
}

Value<std::string> &
OnlineStatementImpl::addIdentifier()
{
    Value<std::string> *tmp = new Value<std::string>(
            Column("ident", joinOnlineStatementTable()));
    add(tmp);
    tmp->setName("Identifier");
    return *tmp;
}

} // namespace Filters
} // namespace Database

