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

namespace Filters {

class RequestServiceType : public Database::Filters::Value<long> {
public:
	RequestServiceType(const long val = 0) : Value<long>() {
		setValue(val);
	}	
	RequestServiceType(const Column &col) : Database::Filters::Value<long>(col) {
	}
/*
	void setValue(RequestServiceType &val) {
		// use Null<long> to exchange value
		setValue( val.getValue());
	}
*/
	
	friend std::ostream& operator<<(std::ostream &_os, const RequestServiceType& _v);
	friend std::istream& operator>>(std::istream &_is, RequestServiceType& _v);
	friend bool operator<(const RequestServiceType &_left, const RequestServiceType &_right);
	friend bool operator>(const RequestServiceType &_left, const RequestServiceType &_right);

	operator long() const {
		return getValue().getValue();
	}

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
            const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_TEMPLATE(
                "Value_long_ServiceType",
                Database::Filters::Value<long>);
    }
};

class RequestActionType : public Database::Filters::Value<long> {
public:
	RequestActionType(const long val = 0) : Value<long>() {
		setValue(val);
	}	
	RequestActionType(const Column &col) : Database::Filters::Value<long>(col) {
	}
	friend std::ostream& operator<<(std::ostream &_os, const RequestActionType& _v); 
	friend std::istream& operator>>(std::istream &_is, RequestActionType& _v);
	friend bool operator<(const RequestActionType &_left, const RequestActionType &_right);
	friend bool operator>(const RequestActionType &_left, const RequestActionType &_right);
	operator long() const {
		return getValue().getValue();
	}

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
            const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_TEMPLATE(
                "Value_long_ActionType",
                Database::Filters::Value<long>);
    }
};

class RequestPropertyValue : virtual public Compound
{
	public:
	virtual ~RequestPropertyValue() {
	}

  virtual Table& joinRequestPropertyValueTable() = 0;
  virtual Table& joinRequestPropertyTable() = 0;
  virtual Value<std::string>& addName() =  0;
  virtual Value<std::string>& addValue() = 0;
    virtual Value<bool>& addOutputFlag() = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
        const unsigned int _version) {
      _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }

    static RequestPropertyValue *create();
};

class RequestPropertyValueImpl : virtual public RequestPropertyValue
{
public:
	RequestPropertyValueImpl(bool set_active = false);
	virtual ~RequestPropertyValueImpl() {
	}

	virtual Table& joinRequestPropertyValueTable();
	virtual Table& joinRequestPropertyTable();
	virtual Value<std::string>& addName();
	virtual Value<std::string>& addValue();
	virtual Value<bool>& addOutputFlag();

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
  virtual Value<std::string>& addUserName() = 0;
  virtual Value<bool>& addIsMonitoring() = 0;
  virtual RequestServiceType& addService() = 0;
  virtual RequestActionType&  addActionType() = 0;
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
  virtual Value<std::string>& addUserName();
  virtual Value<bool>& addIsMonitoring();
  virtual RequestServiceType& addService();
  virtual RequestActionType& addActionType();
  virtual RequestData& addRequestData();
  virtual RequestPropertyValue& addRequestPropertyValue();


  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Request);
  }

};

class Session : virtual public Compound {
public:
  virtual ~Session() {
  };
 
  virtual Table& joinSessionTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Value<std::string>& addName() = 0;
  virtual Interval<Database::DateTimeInterval>& addLoginDate() = 0;
  virtual Interval<Database::DateTimeInterval>& addLogoutDate() = 0;
  virtual Value<std::string>& addLang() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

  static Session* create();
};
  

class SessionImpl : virtual public Session {
public:
  SessionImpl(bool set_action = false);
  virtual ~SessionImpl() {
  }

  virtual Table& joinSessionTable();
  virtual Value<Database::ID>& addId();
  virtual Value<std::string>& addName();
  virtual Interval<Database::DateTimeInterval>& addLoginDate();
  virtual Interval<Database::DateTimeInterval>& addLogoutDate();
  virtual Value<std::string>& addLang();
 
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Session);
  }

};

}

}

#endif
