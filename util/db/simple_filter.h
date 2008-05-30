#ifndef SIMPLE_FILTER_H_
#define SIMPLE_FILTER_H_

#include <string>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>

#include "sql_operators.h"
#include "filter.h"

namespace DBase {

class SelectQuery;

namespace Filters {

class Simple : public Filter {
public:
  Simple() :
    Filter(SQL_OP_AND), allowed_wildcard(true) {
  }
  
  Simple(const std::string& _conj) :
    Filter(_conj), allowed_wildcard(true) {
  }
  
  virtual ~Simple();

  virtual Simple* clone() {
    return 0;
  }

  virtual bool isSimple() const {
    return true;
  }

  virtual bool isActive() const {
    TRACE("[CALL] Simple::isActive()");
    LOGGER("db").debug(boost::format("filter '%1%' is %2%") % getName()
        % (active ? "active" : "not active"));
    return active;
  }

  virtual void enableWildcardExpansion() {
    allowed_wildcard = true;
  }

  virtual void disableWildcardExpansion() {
    allowed_wildcard = false;
  }
  
  // virtual void serialize(DBase::SelectQuery& _sq) = 0;
  virtual void serialize(DBase::SelectQuery& _sq) {
  }

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Filter);
  }

protected:
  bool allowed_wildcard;
};

}
}

#endif /*SIMPLE_FILTER_H_*/
