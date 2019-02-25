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
#include "src/deprecated/model/nsset_filter.hh"

namespace Database {
namespace Filters {

NSSet* NSSet::create() {
  return new NSSetHistoryImpl();
}

/*
 * NSSET IMPLEMENTATION
 */
NSSetImpl::NSSetImpl() :
  ObjectImpl() {
  setName("NSSet");
  addType().setValue(getType());
}

NSSetImpl::~NSSetImpl() {
}

Table& NSSetImpl::joinNSSetTable() {
  return joinTable("nsset");
}

Value<std::string>& NSSetImpl::addHandle() {
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinObjectRegistryTable()));
  add(tmp);
  tmp->addPreValueString("UPPER(");
  tmp->addPostValueString(")");
  tmp->setName("Handle");
  return *tmp;
}

Value<ID>& NSSetImpl::addId() {
  Value<ID> *tmp = new Value<ID>(Column("id", joinNSSetTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}

Value<std::string>& NSSetImpl::addHostFQDN() {
  joins.push_back(new Join(
      Column("id", joinNSSetTable()),
      SQL_OP_EQ,
      Column("nssetid", joinTable("host"))));
  Value<std::string> *tmp = new Value<std::string>(Column("fqdn", joinTable("host")));
  tmp->setName("HostFQDN");
  add(tmp);
  return *tmp;
}

Value<std::string>& NSSetImpl::addHostIP() {
  joins.push_back(new Join(
      Column("id", joinNSSetTable()),
      SQL_OP_EQ,
      Column("nssetid", joinTable("host_ipaddr_map"))));
  Value<std::string> *tmp = new Value<std::string>(Column("ipaddr", joinTable("host_ipaddr_map"), "host"));
  tmp->setName("HostIP");
  add(tmp);
  return *tmp;
}

Contact& NSSetImpl::addTechContact() {
  Contact *tmp = new ContactImpl();
  add(tmp);
  tmp->setName("TechContact");
  tmp->addJoin(new Join(
      Column("contactid", joinTable("nsset_contact_map")),
      SQL_OP_EQ,
      Column("id", tmp->joinObjectRegistryTable())));
  tmp->joinOn(new Join(
      Column("id", joinNSSetTable()),
      SQL_OP_EQ,
      Column("nssetid", joinTable("domain_contact_map"))));
  return *tmp;
}

void NSSetImpl::_joinPolymorphicTables() {
  ObjectImpl::_joinPolymorphicTables();
  Table *n = findTable("nsset");
  if (n) {
    joins.push_front(new Join(
        Column("id", joinTable("object_registry")),
        SQL_OP_EQ,
        Column("id", *n)
    ));
  }
}

/*
 * NSSET HISTORY IMPLEMENTATION
 */
NSSetHistoryImpl::NSSetHistoryImpl() :
  ObjectHistoryImpl() {
  setName("NSSetHistory");
  addType().setValue(getType());
}

NSSetHistoryImpl::~NSSetHistoryImpl() {
}

Table& NSSetHistoryImpl::joinNSSetTable() {
  return joinTable("nsset_history");
}

Value<std::string>& NSSetHistoryImpl::addHandle() {
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinObjectRegistryTable()));
  add(tmp);
  tmp->addPreValueString("UPPER(");
  tmp->addPostValueString(")");
  tmp->setName("Handle");
  return *tmp;
}

Value<ID>& NSSetHistoryImpl::addId() {
  Value<ID> *tmp = new Value<ID>(Column("id", joinNSSetTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}

Value<std::string>& NSSetHistoryImpl::addHostFQDN() {
  joins.push_back(new Join(
      Column("id", joinNSSetTable()),
      SQL_OP_EQ,
      Column("nssetid", joinTable("host_history"))));
  Value<std::string> *tmp = new Value<std::string>(Column("fqdn", joinTable("host_history")));
  tmp->setName("HostFQDN");
  add(tmp);
  return *tmp;
}

Value<std::string>& NSSetHistoryImpl::addHostIP() {
  joins.push_back(new Join(
      Column("id", joinNSSetTable()),
      SQL_OP_EQ,
      Column("nssetid", joinTable("host_ipaddr_map_history"))));
  Value<std::string> *tmp = new Value<std::string>(Column("ipaddr", joinTable("host_ipaddr_map_history"), "host"));
  tmp->setName("HostIP");
  add(tmp);
  return *tmp;
}

Contact& NSSetHistoryImpl::addTechContact() {
  Contact *tmp = Contact::create();
  add(tmp);
  tmp->setName("TechContact");
  tmp->addJoin(new Join(
      Column("id", tmp->joinObjectRegistryTable()),
      SQL_OP_EQ,
      Column("contactid", joinTable("nsset_contact_map_history"))));
  tmp->joinOn(new Join(
      Column("historyid", joinNSSetTable()),
      SQL_OP_EQ,
      Column("historyid", joinTable("nsset_contact_map_history"))));
  return *tmp;
}

void NSSetHistoryImpl::_joinPolymorphicTables() {
  Table *n = findTable("nsset_history");
  if (n) {
    joins.push_front(new Join(
        Column("historyid", joinTable("object_history")),
        SQL_OP_EQ,
        Column("historyid", *n)
    ));
  }
  ObjectHistoryImpl::_joinPolymorphicTables();
}

}
}
