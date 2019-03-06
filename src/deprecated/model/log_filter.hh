/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef LOG_FILTER_HH_B926012F433745A0BB00F1048DD7B776
#define LOG_FILTER_HH_B926012F433745A0BB00F1048DD7B776

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

/*
#include "src/deprecated/model/object_filter.hh"
#include "src/deprecated/model/contact_filter.hh"
#include "src/deprecated/model/nsset_filter.hh"
#include "src/deprecated/model/keyset_filter.hh"
*/

#include "src/util/db/query/base_filters.hh"
#include "util/types/convert_sql_base.hh" 
/*
#include "src/deprecated/model/model_filters.hh"

#include "src/deprecated/model/registrar_filter.hh"
#include "src/deprecated/model/object_state_filter.hh"

#include "util/log/logger.hh"
#include "src/util/settings.hh"
#include "types/conversions.h"
*/


namespace Database {

namespace Filters {

class ServiceType : public Database::Filters::Value<long> {
public:
    using Database::Filters::Value<long>::serialize;

	ServiceType(const long val) : Value<long>() {
		setValue(val);
	}	
	ServiceType(const Column &col) : Database::Filters::Value<long>(col) {
	}
	ServiceType() {
	}
/*
	void setValue(ServiceType &val) {
		// use Null<long> to exchange value
		setValue( val.getValue());
	}
*/
	
	friend std::ostream& operator<<(std::ostream &_os, const ServiceType& _v);
	friend std::istream& operator>>(std::istream &_is, ServiceType& _v);
	friend bool operator<(const ServiceType &_left, const ServiceType &_right);
	friend bool operator>(const ServiceType &_left, const ServiceType &_right);

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

class RequestType : public Database::Filters::Value<long> {
public:
	RequestType(const long val) : Value<long>() {
		setValue(val);
	}	
	RequestType(const Column &col) : Database::Filters::Value<long>(col) {
	}
	RequestType() {
	}

	friend std::ostream& operator<<(std::ostream &_os, const RequestType& _v); 
	friend std::istream& operator>>(std::istream &_is, RequestType& _v);
	friend bool operator<(const RequestType &_left, const RequestType &_right);
	friend bool operator>(const RequestType &_left, const RequestType &_right);
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

class ResultCode : virtual public Compound
{
public:
    virtual ~ResultCode() {
    }

    virtual Table& joinResultCodeTable() = 0;
    virtual Value<Database::ID>& addServiceId() = 0;
    virtual Value<int>& addResultCode() = 0;
    virtual Value<std::string>& addName() = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
        const unsigned int _version) {
      _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }

    static ResultCode *create();
};

class ResultCodeImpl : virtual public ResultCode
{
public:
    ResultCodeImpl(bool set_active = false);
    virtual ~ResultCodeImpl() {
    }


    virtual Table& joinResultCodeTable();
    virtual Value<Database::ID>& addServiceId();
    virtual Value<int>& addResultCode();
    virtual Value<std::string>& addName();

    friend class boost::serialization::access;
	template<class Archive> void serialize(Archive& _ar,
		const unsigned int _version) {
	  _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ResultCode);
	}
};

class RequestObjectRef : virtual public Compound 
{
public:
    virtual ~RequestObjectRef() {
    }
       
    virtual Table& joinRequestObjectRefTable() = 0;
    virtual Value<std::string>& addObjectType() = 0;
    virtual Value<Database::ID>& addObjectId() = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
        const unsigned int _version) {
      _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }

    static RequestObjectRef *create();
};

class RequestObjectRefImpl : virtual public RequestObjectRef
{
public:
    RequestObjectRefImpl(bool set_active = false);
    virtual ~RequestObjectRefImpl() {
    }

    virtual Table& joinRequestObjectRefTable();
    Table& joinRequestObjectTypeTable();
    virtual Value<std::string>& addObjectType();
    virtual Value<Database::ID>& addObjectId();

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
        const unsigned int _version) {
      _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RequestObjectRef);
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
  virtual Value<Database::ID>& addUserId() = 0;
  virtual Value<bool>& addIsMonitoring() = 0;
  virtual ServiceType& addServiceType() = 0;
  virtual RequestType&  addRequestType() = 0;
  virtual RequestData& addRequestData() = 0;
  virtual RequestPropertyValue&   addRequestPropertyValue() = 0;
  virtual ResultCode& addResultCode() = 0;
  virtual RequestObjectRef& addRequestObjectRef() = 0;

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
  virtual Value<Database::ID>& addUserId();
  virtual Value<bool>& addIsMonitoring();
  virtual ServiceType& addServiceType();
  virtual RequestType& addRequestType();
  virtual RequestData& addRequestData();
  virtual RequestPropertyValue& addRequestPropertyValue();
  virtual ResultCode& addResultCode();
  virtual RequestObjectRef& addRequestObjectRef();

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
  virtual Value<std::string>& addUserName() = 0;
  virtual Value<Database::ID>& addUserId() = 0;
  virtual Interval<Database::DateTimeInterval>& addLoginDate() = 0;
  virtual Interval<Database::DateTimeInterval>& addLogoutDate() = 0;

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
  virtual Value<std::string>& addUserName();
  virtual Value<Database::ID>& addUserId();
  virtual Interval<Database::DateTimeInterval>& addLoginDate();
  virtual Interval<Database::DateTimeInterval>& addLogoutDate();
 
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Session);
  }

};

}

}

#endif
