#ifndef EPP_ACTION_FILTER_H_
#define EPP_ACTION_FILTER_H_

#include "db/base_filters.h"
#include "epp_session_filter.h"
#include "object_filter.h"

namespace Database {
namespace Filters {

class EppAction : virtual public Compound {
public:
  virtual ~EppAction() {
  }

  virtual Table& joinActionTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Interval<DateTimeInterval>& addTime() = 0;
  virtual Value<std::string>& addClTRID() = 0;
  virtual Value<std::string>& addSvTRID() = 0;
  virtual Value<int>& addEppCodeResponse() = 0;
  virtual Value<int>& addResponse() = 0;
  virtual Value<int>& addType() = 0;
  virtual EppSession& addSession() = 0;
  virtual Registrar& addRegistrar() = 0;
  virtual Object& addObject() = 0;
  /* filter on requested object handle - it doesn't need to be in register
   * done by using action_elements table  */
  virtual Value<std::string>& addRequestHandle() = 0;
  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

  static EppAction* create();
};

class EppActionImpl : virtual public EppAction {
public:
  EppActionImpl();
  virtual ~EppActionImpl();

  virtual Table& joinActionTable();
  virtual Value<Database::ID>& addId();
  virtual Interval<DateTimeInterval>& addTime();
  virtual Value<std::string>& addClTRID();
  virtual Value<std::string>& addSvTRID();
  virtual Value<int>& addEppCodeResponse();
  virtual Value<int>& addResponse();
  virtual Value<int>& addType();
  virtual EppSession& addSession();
  virtual Registrar& addRegistrar();
  virtual Object& addObject();
  virtual Value<std::string>& addRequestHandle();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EppAction);
  }
};

}
}

#endif /*EPP_ACTION_FILTER_H_*/
