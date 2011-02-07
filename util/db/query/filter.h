#ifndef FILTER_H_
#define FILTER_H_

#include <string>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>

#include "query_old.h"
#include "types/data_types.h"
#include "sql_helper_objects.h"
#include "sql_operators.h"
#include "log/logger.h"

#include "settings.h"
//#include "visitor.h"

namespace Database {

class SelectQuery;

namespace Filters {

/**
 * Base abstract class for query filter framework
 * Top of the Composite pattern
 */
class Filter {
public:
  /**
   * C-tor, D-tor
   */
  Filter() :
    neg(false), active(false) {
  }
  Filter(const std::string& _conj) :
    neg(false), active(false), conj(_conj) {
  }

  /// copy constructor
  Filter(const Filter &f) :
      neg(f.neg), active(f.active), conj(f.conj), name(f.name) {
  }

  virtual ~Filter();
  virtual bool isSimple() const = 0;
  virtual void setAND() {
    conj = SQL_OP_AND;
  }
  virtual void setOR() {
    conj = SQL_OP_OR;
  }
  virtual void toggleNOT() {
    neg = 1 - neg;
  }
  virtual void setNOT(bool _neg) {
    neg = _neg;
  }
  virtual bool getNOT() const {
    return neg;
  }
  virtual const std::string& getName() const {
    return name;
  }
  virtual void setName(const std::string& _name) {
    name = _name;
  }
  virtual bool isActive() const = 0;
  
  /**
   * Query data filling
   */
  virtual void serialize(Database::SelectQuery& _sq, const Settings_* _settings = 0) = 0;
  
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& _ar, const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_NVP(neg);
    _ar & BOOST_SERIALIZATION_NVP(active);
    _ar & BOOST_SERIALIZATION_NVP(conj);
    _ar & BOOST_SERIALIZATION_NVP(name);
  }
  
protected:
  virtual void setConjuction(const std::string& _conj) {
    conj = _conj;
  }
  virtual const std::string getConjuction() const {
    return (neg ? (conj + SQL_OP_NOT) : conj);
  }

  /* filter negation flag (NOT) */
  bool neg;
  /* value was set flag */
  bool active;
  /**
   * conjuction with preceding filter
   * - generally whatever
   * - we use SQL conjuctions (AND OR) 
   */
  std::string conj;
  /* filter naming */
  std::string name;
};

}
}

#endif /*FILTER_H_*/
