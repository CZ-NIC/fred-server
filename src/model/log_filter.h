#ifndef LOG_FILTER_H_
#define LOG_FILTER_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

/*
#include "object_filter.h"
#include "contact_filter.h"
#include "nsset_filter.h"
#include "keyset_filter.h"
*/

#include "db/query/base_filters.h"
#include "types/convert_sql_base.h"

/*
#include "model_filters.h"

#include "registrar_filter.h"
#include "object_state_filter.h"

#include "log/logger.h"
#include "settings.h"
#include "types/conversions.h"
*/

namespace Database {

typedef long RequestServiceType;
typedef long RequestActionType;

namespace Filters {

class RequestProperty : virtual public Compound
{
public:
	virtual ~RequestProperty() {
	}

	virtual Table& joinRequestPropertyTable() = 0;
	virtual Value<std::string>& addName() = 0;

	friend class boost::serialization::access;
	  template<class Archive> void serialize(Archive& _ar, const unsigned int _version) {
	    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
	  }

	  static RequestProperty* create();
};

class RequestPropertyValue : virtual public Compound
{
	public:
	virtual ~RequestPropertyValue() {
	}

  virtual Table& joinRequestPropertyValueTable() = 0;
  virtual Value<std::string>& addValue() = 0;
  virtual Value<Database::ID>& addRequestPropertyId() = 0;
    virtual Value<bool>& addOutputFlag()	= 0;
    virtual RequestProperty& addRequestProperty()  = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
        const unsigned int _version) {
      _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }

    static RequestPropertyValue *create();
};

class RequestPropertyImpl : virtual public RequestProperty
{
public:
	RequestPropertyImpl(bool set_active);
	virtual ~RequestPropertyImpl() {
	}

	virtual Table& joinRequestPropertyTable();
	virtual Value<std::string>& addName();

	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive& _ar,
		const unsigned int _version) {
	  _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RequestProperty);
	}
};

class RequestPropertyValueImpl : virtual public RequestPropertyValue
{
public:
	RequestPropertyValueImpl(bool set_active);
	virtual ~RequestPropertyValueImpl() {
	}

	virtual Table& joinRequestPropertyValueTable();
	virtual Value<std::string>& addValue();
	// virtual Value<Database::ID>& addNameId();
	virtual Value<bool>& addOutputFlag();
	virtual RequestProperty&  addRequestProperty();
	virtual Value<Database::ID>& addRequestPropertyId();

	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive& _ar,
		const unsigned int _version) {
	  _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RequestPropertyValue);
	}
};


class RequestData : virtual public Compound {
public:
	virtual ~RequestData () {
	}

	virtual Table & joinRequestDataTable() = 0;
	virtual Value<std::string>& addContent() = 0;
	virtual Value<bool>& addResponseFlag() = 0;

	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive& _ar,
	    const unsigned int _version) {
	  _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
	}

	static RequestData* create();
};

class RequestDataImpl : virtual public RequestData {
public:
	RequestDataImpl(bool set_active=false);
	virtual ~RequestDataImpl() {
	}

	virtual Table &joinRequestDataTable();
	virtual Value<std::string>& addContent();
	virtual Value<bool>& addResponseFlag();

	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive& _ar,
		const unsigned int _version) {
	  _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RequestData);
	}
};

class Request : virtual public Compound
{
public:

  virtual ~Request() {
  }

  virtual Table& joinRequestTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Interval<Database::DateTimeInterval>& addTimeBegin() = 0;
  virtual Interval<Database::DateTimeInterval>& addTimeEnd() = 0;
  virtual Value<std::string>& addSourceIp() = 0;
  virtual Value<Database::RequestServiceType>& addServiceType() = 0;
  virtual Value<Database::ID>& addRequestDataId()  = 0;
  virtual Value<Database::ID>& addRequestPropertyValueId() = 0;
  virtual RequestData& addRequestData() = 0;
  virtual RequestPropertyValue&   addRequestPropertyValue() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

  static Request* create();
};

class RequestImpl : virtual public Request {
public:
  RequestImpl(bool set_active = false);
  virtual ~RequestImpl() {
  }

  virtual Table& joinRequestTable();
  virtual Value<Database::ID>& addId();
  virtual Interval<Database::DateTimeInterval>& addTimeBegin();
  virtual Interval<Database::DateTimeInterval>& addTimeEnd();
  virtual Value<std::string>& addSourceIp();
  virtual Value<Database::RequestServiceType>& addServiceType();
  virtual Value<Database::ID>& addRequestDataId();
  virtual Value<Database::ID>& addRequestPropertyValueId();
  virtual RequestData& addRequestData();
  virtual RequestPropertyValue& addRequestPropertyValue();


  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Request);
  }

};

}

}

/*
template<>
struct SqlConvert<Database::RequestServiceType> : public NumericsConvertor<int> { };
*/


#endif
