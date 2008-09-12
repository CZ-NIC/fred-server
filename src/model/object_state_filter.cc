#include "object_state_filter.h"

namespace Database {
namespace Filters {

ObjectStateImpl::ObjectStateImpl() {
}

ObjectStateImpl::~ObjectStateImpl() {
}

Table& ObjectStateImpl::joinObjectStateTable() {
  return joinTable("object_state");
}

Value<Database::ID>& ObjectStateImpl::addStateId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("state_id", joinObjectStateTable()));
  add(tmp);
  tmp->setName("StateId");
  return *tmp;
}

Interval<Database::DateTimeInterval>& ObjectStateImpl::addValidFrom() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("valid_from", joinObjectStateTable()));
  add(tmp);
  tmp->setName("ValidFrom");
  return *tmp;
}

Interval<Database::DateTimeInterval>& ObjectStateImpl::addValidTo() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("valid_to", joinObjectStateTable()));
  add(tmp);
  tmp->setName("ValidTo");
  return *tmp;
}

}
}
