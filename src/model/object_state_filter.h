#ifndef OBJECT_STATE_FILTER_H_
#define OBJECT_STATE_FILTER_H_

#include "db/base_filters.h"

namespace Database {
namespace Filters {

class ObjectState : virtual public Compound {
public:
  virtual ~ObjectState() {
  }

  virtual Table& joinObjectStateTable() = 0;
  virtual Value<Database::ID>& addStateId() = 0;
  virtual Interval<Database::DateTimeInterval>& addValidFrom() = 0;
  virtual Interval<Database::DateTimeInterval>& addValidTo() = 0;
  //TODO: more methods
};

class ObjectStateImpl : virtual public ObjectState {
public:
  ObjectStateImpl();
  virtual ~ObjectStateImpl();

  virtual Table& joinObjectStateTable();
  virtual Value<Database::ID>& addStateId();
  virtual Interval<Database::DateTimeInterval>& addValidFrom();
  virtual Interval<Database::DateTimeInterval>& addValidTo();

  void serialize(SelectQuery& _sq, const Settings *_settings) {
    std::string history = (_settings ? _settings->get("filter.history") : "not_set");
    LOGGER("db").debug(boost::format("attribute `filter.history' is set to `%1%'")
                                     % history);
    if (history == "off" || history == "not_set") {
      addValidTo().setNULL();
    }
    Compound::serialize(_sq, _settings);
  }

};

}
}

#endif /*OBJECT_STATE_FILTER_H_*/
