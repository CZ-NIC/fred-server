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

#include "db/base_filters.h"

#include "types/data_types.h"

/*
#include "model_filters.h"

#include "registrar_filter.h"
#include "object_state_filter.h"

#include "log/logger.h"
#include "settings.h"
#include "types/conversions.h"
*/

namespace Database {

enum LogServiceType { LC_UNIX_WHOIS, LC_WEB_WHOIS, LC_EPP, LC_WEBADMIN };

namespace Filters {

class LogPropertyName : virtual public Compound
{
public:
	virtual ~LogPropertyName() {
	}

	virtual Table& joinLogPropertyNameTable() = 0;
	virtual Value<std::string>& addName() = 0;

	friend class boost::serialization::access;
	  template<class Archive> void serialize(Archive& _ar,
	      const unsigned int _version) {
	    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
	  }

	  static LogPropertyName* create();
};

class LogPropertyValue : virtual public Compound
{
	public:
	virtual ~LogPropertyValue() {
	}

  virtual Table& joinLogPropertyValueTable() = 0;
  virtual Value<std::string>& addValue() = 0;
  virtual Value<Database::ID>& addLogPropertyNameId() = 0;
    virtual Value<bool>& addOutputFlag()	= 0;
    virtual LogPropertyName& addLogPropertyName()  = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
        const unsigned int _version) {
      _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }

    static LogPropertyValue *create();
};

class LogPropertyNameImpl : virtual public LogPropertyName
{
public:
	LogPropertyNameImpl(bool set_active);
	virtual ~LogPropertyNameImpl() {
	}

	virtual Table& joinLogPropertyNameTable();
	virtual Value<std::string>& addName();

	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive& _ar,
		const unsigned int _version) {
	  _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(LogPropertyName);
	}
};

class LogPropertyValueImpl : virtual public LogPropertyValue
{
public:
	LogPropertyValueImpl(bool set_active);
	virtual ~LogPropertyValueImpl() {
	}

	virtual Table& joinLogPropertyValueTable();
	virtual Value<std::string>& addValue();
	// virtual Value<Database::ID>& addNameId();
	virtual Value<bool>& addOutputFlag();
	virtual LogPropertyName&  addLogPropertyName();
	virtual Value<Database::ID>& addLogPropertyNameId();

	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive& _ar,
		const unsigned int _version) {
	  _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(LogPropertyValue);
	}
};


class LogRawContent : virtual public Compound {
public:
	virtual ~LogRawContent () {
	}

	virtual Table & joinLogRawContentTable() = 0;
	virtual Value<std::string>& addRequest() = 0;
	virtual Value<std::string>& addResponse() = 0;

	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive& _ar,
	    const unsigned int _version) {
	  _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
	}

	static LogRawContent* create();
};

class LogRawContentImpl : virtual public LogRawContent {
public:
	LogRawContentImpl(bool set_active=0);
	virtual ~LogRawContentImpl() {
	}

	virtual Table &joinLogRawContentTable();
	virtual Value<std::string>& addRequest();
	virtual Value<std::string>& addResponse();

	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive& _ar,
		const unsigned int _version) {
	  _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(LogRawContent);
	}
};

class LogEntry : virtual public Compound
{
public:

  virtual ~LogEntry() {
  }

  virtual Table& joinLogEntryTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Interval<Database::DateTimeInterval>& addTimeBegin() = 0;
  virtual Interval<Database::DateTimeInterval>& addTimeEnd() = 0;
  virtual Value<std::string>& addSourceIp() = 0;
  virtual Value<Database::LogServiceType>& addServiceType() = 0;
  virtual Value<Database::ID>& addLogRawContentId()  = 0;
  virtual Value<Database::ID>& addLogPropertyValueId() = 0;
  virtual LogRawContent& addLogRawContent() = 0;
  virtual LogPropertyValue&   addLogPropertyValue() = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

  static LogEntry* create();
};

class LogEntryImpl : virtual public LogEntry {
public:
  LogEntryImpl(bool set_active);
  virtual ~LogEntryImpl() {
  }

  virtual Table& joinLogEntryTable();
  virtual Value<Database::ID>& addId();
  virtual Interval<Database::DateTimeInterval>& addTimeBegin();
  virtual Interval<Database::DateTimeInterval>& addTimeEnd();
  virtual Value<std::string>& addSourceIp();
  virtual Value<Database::LogServiceType>& addServiceType();
  virtual Value<Database::ID>& addLogRawContentId();
  virtual Value<Database::ID>& addLogPropertyValueId();
  virtual LogRawContent& addLogRawContent();
  virtual LogPropertyValue& addLogPropertyValue();


  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(LogEntry);
  }

};

}

CONVERSION_DECLARATION(Database::LogServiceType)

inline Database::LogServiceType Conversion<Database::LogServiceType>::from_string(const std::string& _value) {
  return (Database::LogServiceType)atoi(_value.c_str());
}

inline std::string Conversion<Database::LogServiceType>::to_string(const Database::LogServiceType& _value) {
  return signed2string((int)_value);
}

}

#endif
