/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
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
#include "src/deprecated/model/zone_ns_filter.hh"

namespace Database {
namespace Filters {

ZoneNsImpl::ZoneNsImpl(bool set_active)
	: Compound()
{
    setName("ZoneNs");
    active = set_active;
}

ZoneNsImpl::~ZoneNsImpl()
{ }

Table &
ZoneNsImpl::joinZoneNsTable()
{
    return joinTable("zone_ns");
}

Value<Database::ID> &
ZoneNsImpl::addId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(Column("id", joinZoneNsTable()));
    add(tmp);
    tmp->setName("Id");
    return *tmp;
}

Value<Database::ID> &
ZoneNsImpl::addZoneId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(Column("zone", joinZoneNsTable()));
    add(tmp);
    tmp->setName("ZoneId");
    return *tmp;
}

Value<std::string> &
ZoneNsImpl::addFqdn()
{
    Value<std::string> *tmp = new Value<std::string>(Column("fqdn", joinZoneNsTable()));
    add(tmp);
    tmp->setName("Fqdn");
    return *tmp;
}

Value<std::string> &
ZoneNsImpl::addAddrs()
{
    Value<std::string> *tmp = new Value<std::string>(Column("addrs", joinZoneNsTable()));
    add(tmp);
    tmp->setName("Addrs");
    return *tmp;
}

} // namespace Filters
} // namespace Database
