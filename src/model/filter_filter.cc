#include "filter_filter.h"

namespace DBase {
namespace Filters {

FilterFilterImpl::FilterFilterImpl() {
  setName("Filter");
}

FilterFilterImpl::~FilterFilterImpl() {
}

Table& FilterFilterImpl::joinFilterTable() {
  return joinTable("filters");
}

Value<DBase::ID>& FilterFilterImpl::addId() {
  Value<DBase::ID> *tmp = new Value<ID>(Column("id", joinFilterTable()));
  add(tmp);
  tmp->setName("Id");
  return *tmp;
}

Value<DBase::ID>& FilterFilterImpl::addUserId() {
  Value<DBase::ID> *tmp = new Value<ID>(Column("userid", joinFilterTable()));
  add(tmp);
  tmp->setName("UserId");
  return *tmp;
}

Value<DBase::ID>& FilterFilterImpl::addGroupId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("groupid", joinFilterTable()));
  add(tmp);
  tmp->setName("GroupId");
  return *tmp;
}

Value<int>& FilterFilterImpl::addType() {
  Value<int> *tmp = new Value<int>(Column("type", joinFilterTable()));
  add(tmp);
  tmp->setName("Type");
  return *tmp;
}

}
}
