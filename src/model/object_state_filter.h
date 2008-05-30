#ifndef OBJECT_STATE_FILTER_H_
#define OBJECT_STATE_FILTER_H_

#include "db/base_filters.h"

namespace DBase {
namespace Filters {

class ObjectState : virtual public Compound {
public:
  virtual ~ObjectState() {
  }

  virtual Table& joinObjectStateTable() = 0;
  virtual Value<int>& addId() = 0;
  virtual Interval<DBase::DateTimeInterval>& addValidFrom() = 0;
  virtual Interval<DBase::DateTimeInterval>& addValidTo() = 0;
  //TODO: more methods
};

class ObjectStateImpl : virtual public ObjectState {
public:
  ObjectStateImpl();
  virtual ~ObjectStateImpl();

  virtual Table& joinObjectStateTable();
  virtual Value<int>& addId();
  virtual Interval<DBase::DateTimeInterval>& addValidFrom();
  virtual Interval<DBase::DateTimeInterval>& addValidTo();
};

}
}

#endif /*OBJECT_STATE_FILTER_H_*/
