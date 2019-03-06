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
#include "src/deprecated/model/zone_filter.hh"

namespace Database {
namespace Filters {

ZoneImpl::ZoneImpl(bool set_active)
	: Compound()
{
    setName("Zone");
    active = set_active;
}

ZoneImpl::~ZoneImpl()
{ }

Table &
ZoneImpl::joinZoneTable()
{
    return joinTable("zone");
}

Value<Database::ID> &
ZoneImpl::addId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(Column("id", joinZoneTable()));
    add(tmp);
    tmp->setName("Id");
    return *tmp;
}

Value<std::string> &
ZoneImpl::addFqdn()
{
    Value<std::string> *tmp = new Value<std::string>(Column("fqdn", joinZoneTable()));
    add(tmp);
    tmp->setName("Fqdn");
    return *tmp;
}

Value<int> &
ZoneImpl::addExPeriodMin()
{
    Value<int> *tmp = new Value<int>(Column("ex_period_min", joinZoneTable()));
    add(tmp);
    tmp->setName("ExPeriodMin");
    return *tmp;
}

Value<int> &
ZoneImpl::addExPeriodMax()
{
    Value<int> *tmp = new Value<int>(Column("ex_period_max", joinZoneTable()));
    add(tmp);
    tmp->setName("ExPeriodMax");
    return *tmp;
}

Value<int> &
ZoneImpl::addValPeriod()
{
    Value<int> *tmp = new Value<int>(Column("val_period", joinZoneTable()));
    add(tmp);
    tmp->setName("ValPeriod");
    return *tmp;
}

Value<int> &
ZoneImpl::addDotsMax()
{
    Value<int> *tmp = new Value<int>(Column("dots_max", joinZoneTable()));
    add(tmp);
    tmp->setName("DotsMax");
    return *tmp;
}

Value<bool> &
ZoneImpl::addEnumZone()
{
    Value<bool> *tmp = new Value<bool>(Column("enum_zone", joinZoneTable()));
    add(tmp);
    tmp->setName("EnumZone");
    return *tmp;
}

ZoneNs& ZoneImpl::addZoneNs() {
  ZoneNs *tmp = new ZoneNsImpl();
  tmp->joinOn(new Join(Column("id", joinZoneTable()), SQL_OP_EQ, Column("zone", tmp->joinZoneNsTable())));
  add(tmp);
  return *tmp;
}


} // namespace Filters
} // namespace Database
