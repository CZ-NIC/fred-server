#include "object_filter.h"

namespace DBase {
namespace Filters {

/*
 * OBJECT IMPLEMENTATION
 */
ObjectImpl::ObjectImpl() :
  ObjectRegistryImpl() {
  setName("Object");
}

ObjectImpl::~ObjectImpl() {
}

Interval<DBase::DateTimeInterval>& ObjectImpl::addTransferTime() {
  Interval<DBase::DateTimeInterval> *tmp = new Interval<DBase::DateTimeInterval>(Column("trdate", joinObjectTable()));
  add(tmp);
  tmp->setName("TransferTime");
  return *tmp;
}

Interval<DBase::DateTimeInterval>& ObjectImpl::addUpdateTime() {
  Interval<DBase::DateTimeInterval> *tmp = new Interval<DBase::DateTimeInterval>(Column("update", joinObjectTable()));
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

Value<DBase::ID>& ObjectImpl::addRegistrarId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("clid", joinObjectTable()));
  add(tmp);
  tmp->setName("RegistrarId");
  return *tmp;
}

Value<DBase::ID>& ObjectImpl::addUpdateRegistrarId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("upid", joinObjectTable()));
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
  tmp->setName("UpdateRegistar");
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
  Compound::_joinPolymorphicTables();
}

/*
 * OBJECT HISTORY IMPLEMENTATION
 */
ObjectHistoryImpl::ObjectHistoryImpl() : ObjectRegistryImpl() {
  setName("ObjectHistory");
}

ObjectHistoryImpl::~ObjectHistoryImpl() {
}

Interval<DBase::DateTimeInterval>& ObjectHistoryImpl::addTransferTime() {
  Interval<DBase::DateTimeInterval> *tmp = new Interval<DBase::DateTimeInterval>(Column("trdate", joinObjectTable()));
  add(tmp);
  tmp->setName("TransferTime");
  return *tmp;
}

Interval<DBase::DateTimeInterval>& ObjectHistoryImpl::addUpdateTime() {
  Interval<DBase::DateTimeInterval> *tmp = new Interval<DBase::DateTimeInterval>(Column("update", joinObjectTable()));
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

Value<DBase::ID>& ObjectHistoryImpl::addRegistrarId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("clid", joinObjectTable()));
  add(tmp);
  tmp->setName("RegistrarId");
  return *tmp;
}

Value<DBase::ID>& ObjectHistoryImpl::addUpdateRegistrarId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("upid", joinObjectTable()));
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
  tmp->setName("UpdateRegistar");
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
          Column("historyid", joinTable("object_registry")),
          SQL_OP_EQ,
          Column("historyid", *o)
      ));
    }
  }
  Compound::_joinPolymorphicTables();
}

}
}
