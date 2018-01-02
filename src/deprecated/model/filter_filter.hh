#ifndef FILTER_FILTER_HH_1EFB25A32ABF49B1AF70B2A917F7E91B
#define FILTER_FILTER_HH_1EFB25A32ABF49B1AF70B2A917F7E91B

#include "src/util/db/query/base_filters.hh"

namespace Database {
namespace Filters {

class FilterFilter : virtual public Compound {
public:
  virtual ~FilterFilter() {
  }

  virtual Table& joinFilterTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Value<Database::ID>& addUserId() = 0;
  virtual Value<Database::ID>& addGroupId() = 0;
  virtual Value<int>& addType() = 0;
};

class FilterFilterImpl : virtual public FilterFilter {
public:
  FilterFilterImpl();
  virtual ~FilterFilterImpl();

  virtual Table& joinFilterTable();
  virtual Value<Database::ID>& addId();
  virtual Value<Database::ID>& addUserId();
  virtual Value<Database::ID>& addGroupId();
  virtual Value<int>& addType();
};

}
}

#endif
