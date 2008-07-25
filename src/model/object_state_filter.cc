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

Value<int>& ObjectStateImpl::addId() {
  Value<int> *tmp = new Value<int>(Column("state_id", joinObjectStateTable()));
  add(tmp);
  return *tmp;
}

Interval<Database::DateTimeInterval>& ObjectStateImpl::addValidFrom() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("valid_from", joinObjectStateTable()));
  add(tmp);
  return *tmp;
}

Interval<Database::DateTimeInterval>& ObjectStateImpl::addValidTo() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("valid_to", joinObjectStateTable()));
  add(tmp);
  return *tmp;
}

}
}
