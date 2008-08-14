#include "object_registry_filter.h"

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

