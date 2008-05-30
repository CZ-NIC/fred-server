#ifndef FILTER_FILTER_H_
#define FILTER_FILTER_H_

#include "db/base_filters.h"

namespace DBase {
namespace Filters {

class FilterFilter : virtual public Compound {
public:
  virtual ~FilterFilter() {
  }

  virtual Table& joinFilterTable() = 0;
  virtual Value<DBase::ID>& addId() = 0;
  virtual Value<DBase::ID>& addUserId() = 0;
  virtual Value<DBase::ID>& addGroupId() = 0;
  virtual Value<int>& addType() = 0;
};

class FilterFilterImpl : virtual public FilterFilter {
public:
  FilterFilterImpl();
  virtual ~FilterFilterImpl();

  virtual Table& joinFilterTable();
  virtual Value<DBase::ID>& addId();
  virtual Value<DBase::ID>& addUserId();
  virtual Value<DBase::ID>& addGroupId();
  virtual Value<int>& addType();
};

}
}

#endif
