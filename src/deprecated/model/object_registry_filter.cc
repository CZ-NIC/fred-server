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
#include "src/deprecated/model/object_registry_filter.hh"

namespace Database {
namespace Filters {

ObjectRegistry* ObjectRegistry::create() {
  return new ObjectRegistryImpl();
}


ObjectRegistryImpl::ObjectRegistryImpl() :
  ObjectRegistry() {
  setName("ObjectRegistry");
}

ObjectRegistryImpl::~ObjectRegistryImpl() {
}

Table& ObjectRegistryImpl::joinObjectRegistryTable() {
  return joinTable("object_registry");
}

Value<ObjectType>& ObjectRegistryImpl::addType() {
  Value<ObjectType> *tmp = new Value<ObjectType>(Column("type", joinObjectRegistryTable()));
  add(tmp);
  tmp->setName("Type");
  return *tmp;
}

Value<std::string>& ObjectRegistryImpl::addHandle() {
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinObjectRegistryTable()));
  add(tmp);
  tmp->setName("Handle");
  return *tmp;
}

Interval<Database::DateTimeInterval>& ObjectRegistryImpl::addCreateTime() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("crdate", joinObjectRegistryTable()));
  add(tmp);
  tmp->setName("CreateTime");
  return *tmp;
}

Interval<Database::DateTimeInterval>& ObjectRegistryImpl::addDeleteTime() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("erdate", joinObjectRegistryTable()));
  add(tmp);
  tmp->setName("DeleteTime");
  return *tmp;
}

Registrar& ObjectRegistryImpl::addCreateRegistrar() {
  RegistrarImpl* tmp = new RegistrarImpl();
  add(tmp);
  tmp->joinOn(new Join(Column("crid", joinObjectRegistryTable()), SQL_OP_EQ, Column("id", tmp->joinRegistrarTable())));
  tmp->setName("CreateRegistrar");
  return *tmp;
}

ObjectState& ObjectRegistryImpl::addState() {
  ObjectStateImpl *tmp = new ObjectStateImpl();
  add(tmp);
  tmp->joinOn(new Join(Column("id", joinObjectRegistryTable()), SQL_OP_EQ, Column("object_id", tmp->joinObjectStateTable())));
  tmp->setName("State");
  return *tmp;
}

}
}

