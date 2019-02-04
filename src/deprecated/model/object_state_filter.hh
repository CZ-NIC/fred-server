#ifndef OBJECT_STATE_FILTER_HH_DEF317EB2335491A9AFBEF6B592EBF53
#define OBJECT_STATE_FILTER_HH_DEF317EB2335491A9AFBEF6B592EBF53

#include "src/util/db/query/base_filters.hh"

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
  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar, const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

};

class ObjectStateImpl : virtual public ObjectState {
public:
  ObjectStateImpl();
  virtual ~ObjectStateImpl();

  virtual Table& joinObjectStateTable();
  virtual Value<Database::ID>& addStateId();
  virtual Interval<Database::DateTimeInterval>& addValidFrom();
  virtual Interval<Database::DateTimeInterval>& addValidTo();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar, const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ObjectState);
  }

  void serialize(SelectQuery& _sq, const Settings *_settings) {
    std::string history = (_settings ? _settings->get("filter.history") : "not_set");
    LOGGER.debug(boost::format("attribute `filter.history' is set to `%1%'")
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
