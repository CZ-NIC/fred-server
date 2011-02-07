#ifndef SIMPLE_FILTER_H_
#define SIMPLE_FILTER_H_

#include <string>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>

#include "sql_operators.h"
#include "filter.h"

namespace Database {

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
  
  Simple(const Simple &s) : Filter(s),
          allowed_wildcard(s.allowed_wildcard),
          value_post_(s.value_post_),
          value_pre_(s.value_pre_) {
  }

  virtual ~Simple();

  virtual bool isSimple() const {
    return true;
  }

  virtual bool isActive() const {
    TRACE("[CALL] Simple::isActive()");
    LOGGER(PACKAGE).debug(boost::format("filter '%1%' is %2%") % getName()
        % (active ? "active" : "not active"));
    return active;
  }

  virtual void enableWildcardExpansion() {
    allowed_wildcard = true;
  }

  virtual void disableWildcardExpansion() {
    allowed_wildcard = false;
  }
  
  virtual void serialize(Database::SelectQuery& _sq, const Settings *_settings) {
  }
  
  virtual void addPostValueString(const std::string& _str) {
    value_post_ = _str;
  }

  virtual void addPreValueString(const std::string& _str) {
    value_pre_ = _str;
  }

  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Filter);
    _ar & BOOST_SERIALIZATION_NVP(allowed_wildcard);
    _ar & BOOST_SERIALIZATION_NVP(value_post_);
    _ar & BOOST_SERIALIZATION_NVP(value_pre_);
  }

protected:
  bool allowed_wildcard;
  std::string value_post_;
  std::string value_pre_;
};

}
}

#endif /*SIMPLE_FILTER_H_*/
