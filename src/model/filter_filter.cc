#include "filter_filter.h"

namespace Database {
namespace Filters {

FilterFilterImpl::FilterFilterImpl() {
  setName("Filter");
}

FilterFilterImpl::~FilterFilterImpl() {
}

Table& FilterFilterImpl::joinFilterTable() {
  return joinTable("filters");
}

Value<Database::ID>& FilterFilterImpl::addId() {
  Value<Database::ID> *tmp = new Value<ID>(Column("id", joinFilterTable()));
  add(tmp);
  tmp->setName("Id");
  return *tmp;
}

Value<Database::ID>& FilterFilterImpl::addUserId() {
  Value<Database::ID> *tmp = new Value<ID>(Column("userid", joinFilterTable()));
  add(tmp);
  tmp->setName("UserId");
  return *tmp;
}

Value<Database::ID>& FilterFilterImpl::addGroupId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("groupid", joinFilterTable()));
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
