#include "object_state_filter.h"

namespace DBase {
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

Interval<DBase::DateTimeInterval>& ObjectStateImpl::addValidFrom() {
  Interval<DBase::DateTimeInterval> *tmp = new Interval<DBase::DateTimeInterval>(Column("valid_from", joinObjectStateTable()));
  add(tmp);
  return *tmp;
}

Interval<DBase::DateTimeInterval>& ObjectStateImpl::addValidTo() {
  Interval<DBase::DateTimeInterval> *tmp = new Interval<DBase::DateTimeInterval>(Column("valid_to", joinObjectStateTable()));
  add(tmp);
  return *tmp;
}

}
}
