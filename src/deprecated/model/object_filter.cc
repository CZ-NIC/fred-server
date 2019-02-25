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
#include "src/deprecated/model/object_filter.hh"

namespace Database {
namespace Filters {

Object* Object::create() {
  return new ObjectHistoryImpl();
}

/*
 * OBJECT IMPLEMENTATION
 */
ObjectImpl::ObjectImpl() :
  ObjectRegistryImpl() {
  setName("Object");
}

ObjectImpl::~ObjectImpl() {
}

Interval<Database::DateTimeInterval>& ObjectImpl::addTransferTime() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("trdate", joinObjectTable()));
  add(tmp);
  tmp->setName("TransferTime");
  return *tmp;
}

Interval<Database::DateTimeInterval>& ObjectImpl::addUpdateTime() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("update", joinObjectTable()));
  add(tmp);
  tmp->setName("UpdateTime");
  return *tmp;
}

Value<std::string>& ObjectImpl::addAuthInfo() {
  Value<std::string> *tmp = new Value<std::string>(Column("authinfopw", joinObjectTable()));
  add(tmp);
  tmp->setName("AuthInfo");
  return *tmp;
}

Value<Database::ID>& ObjectImpl::addRegistrarId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("clid", joinObjectTable()));
  add(tmp);
  tmp->setName("RegistrarId");
  return *tmp;
}

Value<Database::ID>& ObjectImpl::addUpdateRegistrarId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("upid", joinObjectTable()));
  add(tmp);
  tmp->setName("UpdateRegistrarId");
  return *tmp;
}

Registrar& ObjectImpl::addRegistrar() { 
  Registrar *tmp = new RegistrarImpl();
  add(tmp);
  tmp->joinOn(new Join(Column("clid", joinObjectTable()), SQL_OP_EQ, Column("id", tmp->joinRegistrarTable())));
  tmp->setName("Registrar");
  return *tmp;
}

Registrar& ObjectImpl::addUpdateRegistrar() {
  RegistrarImpl* tmp = new RegistrarImpl();
  add(tmp);
  tmp->joinOn(new Join(Column("upid", joinObjectTable()), SQL_OP_EQ, Column("id", tmp->joinRegistrarTable())));
  tmp->setName("UpdateRegistrar");
  return *tmp;
}

ObjectState& ObjectImpl::addObjectState() {
  ObjectStateImpl *tmp = new ObjectStateImpl();
  add(tmp);
  tmp->joinOn(new Join(Column("id", joinObjectTable()), SQL_OP_EQ, Column("object_id", tmp->joinObjectStateTable())));
  tmp->setName("ObjectState");
  return *tmp;
}

Table& ObjectImpl::joinObjectTable() {
  return joinTable("object");
}

void ObjectImpl::_joinPolymorphicTables() {
  Table *obr = findTable("object_registry");
  if (obr) {
    Table *o = findTable("object");
    if (o) {
      joins.push_front(new Join(
          Column("id", joinTable("object_registry")),
          SQL_OP_EQ,
          Column("id", *o)
      ));
    }
  }
}

/*
 * OBJECT HISTORY IMPLEMENTATION
 */
ObjectHistoryImpl::ObjectHistoryImpl() : ObjectRegistryImpl() {
  setName("ObjectHistory");
}

ObjectHistoryImpl::~ObjectHistoryImpl() {
}

Value<Database::ID>& ObjectHistoryImpl::addHistoryId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("historyid", joinObjectTable()));
  add(tmp);
  tmp->setName("HistoryId");
  return *tmp;
}

Interval<Database::DateTimeInterval>& ObjectHistoryImpl::addTransferTime() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("trdate", joinObjectTable()));
  add(tmp);
  tmp->setName("TransferTime");
  return *tmp;
}

Interval<Database::DateTimeInterval>& ObjectHistoryImpl::addUpdateTime() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("update", joinObjectTable()));
  add(tmp);
  tmp->setName("UpdateTime");
  return *tmp;
}

Value<std::string>& ObjectHistoryImpl::addAuthInfo() {
  Value<std::string> *tmp = new Value<std::string>(Column("authinfopw", joinObjectTable()));
  add(tmp);
  tmp->setName("AuthInfo");
  return *tmp;
}

Value<Database::ID>& ObjectHistoryImpl::addRegistrarId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("clid", joinObjectTable()));
  add(tmp);
  tmp->setName("RegistrarId");
  return *tmp;
}

Value<Database::ID>& ObjectHistoryImpl::addUpdateRegistrarId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("upid", joinObjectTable()));
  add(tmp);
  tmp->setName("UpdateRegistrarId");
  return *tmp;
}

Registrar& ObjectHistoryImpl::addRegistrar() { 
  Registrar *tmp = new RegistrarImpl();
  add(tmp);
  tmp->joinOn(new Join(Column("clid", joinObjectTable()), SQL_OP_EQ, Column("id", tmp->joinRegistrarTable())));
  tmp->setName("Registrar");
  return *tmp;
}

Registrar& ObjectHistoryImpl::addUpdateRegistrar() {
  RegistrarImpl* tmp = new RegistrarImpl();
  add(tmp);
  tmp->joinOn(new Join(Column("upid", joinObjectTable()), SQL_OP_EQ, Column("id", tmp->joinRegistrarTable())));
  tmp->setName("UpdateRegistrar");
  return *tmp;
}

ObjectState& ObjectHistoryImpl::addObjectState() {
  ObjectStateImpl *tmp = new ObjectStateImpl();
  add(tmp);
  tmp->joinOn(new Join(Column("id", joinObjectTable()), SQL_OP_EQ, Column("object_id", tmp->joinObjectStateTable())));
  tmp->setName("ObjectState");
  return *tmp;
}

Table& ObjectHistoryImpl::joinObjectTable() {
  return joinTable("object_history");
}

void ObjectHistoryImpl::_joinPolymorphicTables() {
  Table *obr = findTable("object_registry");
  if (obr) {
    Table *o = findTable("object_history");
    if (o) {
      joins.push_front(new Join(
          Column("id", joinTable("object_registry")),
          SQL_OP_EQ,
          Column("id", *o)
      ));
    }
  }
}

}
}
