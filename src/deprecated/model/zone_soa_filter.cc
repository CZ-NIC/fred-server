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
#include "src/deprecated/model/zone_soa_filter.hh"

namespace Database {
namespace Filters {

ZoneSoaImpl::ZoneSoaImpl(bool set_active)
	: ZoneImpl()
{
    setName("ZoneSoa");
    active = set_active;
}

ZoneSoaImpl::~ZoneSoaImpl()
{ }

Table &
ZoneSoaImpl::joinZoneSoaTable()
{
    return joinTable("zone_soa");
}


void ZoneSoaImpl::_joinPolymorphicTables()
{
  Table *d = findTable("zone_soa");
  if (d)
  {
    joins.push_back(new Join(
        Column("id", joinTable("zone")),
        SQL_OP_EQ,
        Column("zone", *d)
    ));
  }
}

Value<Database::ID> &
ZoneSoaImpl::addZoneId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(Column("id", joinZoneSoaTable()));
    add(tmp);
    tmp->setName("Id");
    return *tmp;
}

Value<int> &
ZoneSoaImpl::addTtl()
{
    Value<int> *tmp = new Value<int>(Column("ttl", joinZoneSoaTable()));
    add(tmp);
    tmp->setName("Ttl");
    return *tmp;
}

Value<std::string> &
ZoneSoaImpl::addHostmaster()
{
    Value<std::string> *tmp = new Value<std::string>(Column("hostmaster", joinZoneSoaTable()));
    add(tmp);
    tmp->setName("Hostmaster");
    return *tmp;
}

Value<int> &
ZoneSoaImpl::addSerial()
{
    Value<int> *tmp = new Value<int>(Column("serial", joinZoneSoaTable()));
    add(tmp);
    tmp->setName("Serial");
    return *tmp;
}

Value<int> &
ZoneSoaImpl::addRefresh()
{
    Value<int> *tmp = new Value<int>(Column("refresh", joinZoneSoaTable()));
    add(tmp);
    tmp->setName("Refresh");
    return *tmp;
}

Value<int> &
ZoneSoaImpl::addUpdateRetr()
{
    Value<int> *tmp = new Value<int>(Column("update_retr", joinZoneSoaTable()));
    add(tmp);
    tmp->setName("UpdateRetr");
    return *tmp;
}

Value<int> &
ZoneSoaImpl::addExpiry()
{
    Value<int> *tmp = new Value<int>(Column("expiry", joinZoneSoaTable()));
    add(tmp);
    tmp->setName("Expiry");
    return *tmp;
}

Value<int> &
ZoneSoaImpl::addMinimum()
{
    Value<int> *tmp = new Value<int>(Column("minimum", joinZoneSoaTable()));
    add(tmp);
    tmp->setName("Minimum");
    return *tmp;
}

Value<std::string> &
ZoneSoaImpl::addNsFqdn()
{
    Value<std::string> *tmp = new Value<std::string>(Column("ns_fqdn", joinZoneSoaTable()));
    add(tmp);
    tmp->setName("NsFqdn");
    return *tmp;
}

} // namespace Filters
} // namespace Database
