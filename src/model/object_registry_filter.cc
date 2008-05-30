#include "object_registry_filter.h"

namespace DBase {
namespace Filters {

ObjectRegistryImpl::ObjectRegistryImpl() :
  ObjectRegistry() {
  setName("ObjectRegistry");
}

ObjectRegistryImpl::~ObjectRegistryImpl() {
}

Table& ObjectRegistryImpl::joinObjectRegistryTable() {
  return joinTable("object_registry");
}

Value<ObjectType>& ObjectRegistryImpl::setType(const DBase::Null<ObjectType> _type) {
  Value<ObjectType> *tmp = new Value<ObjectType>(Column("type", joinObjectRegistryTable()), _type);
  add(tmp);
  tmp->setName("Type");
  tmp->setValue(_type);
  return *tmp;
}

Value<std::string>& ObjectRegistryImpl::addHandle() {
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinObjectRegistryTable()));
  add(tmp);
  tmp->setName("Handle");
  return *tmp;
}

Interval<DBase::DateTimeInterval>& ObjectRegistryImpl::addCreateTime() {
  Interval<DBase::DateTimeInterval> *tmp = new Interval<DBase::DateTimeInterval>(Column("crdate", joinObjectRegistryTable()));
  add(tmp);
  tmp->setName("CreateTime");
  return *tmp;
}

Interval<DBase::DateTimeInterval>& ObjectRegistryImpl::addDeleteTime() {
  Interval<DBase::DateTimeInterval> *tmp = new Interval<DBase::DateTimeInterval>(Column("erdate", joinObjectRegistryTable()));
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
