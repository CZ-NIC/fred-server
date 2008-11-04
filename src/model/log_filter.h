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

enum LogEventType { LT_REQUEST, LT_RESPONSE };
enum LogComponent { LC_UNIX_WHOIS, LC_WEB_WHOIS, LC_EPP };


namespace Filters {

class LogProperty : virtual public Compound 
{
  public: 
  virtual ~LogProperty() {
  }

  virtual Table& joinLogPropertyTable() = 0;
  virtual Value<std::string>& addValue() = 0; 
  virtual Value<std::string>& addName()  = 0;

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

  static LogProperty* create();
};

class LogPropertyImpl : virtual public LogProperty
{

public:
  LogPropertyImpl(bool set_active); 
  virtual ~LogPropertyImpl() {
  }

  virtual Table& joinLogPropertyTable();
  virtual Value<std::string>& addValue(); 
  virtual Value<std::string>& addName(); 
};

class LogEntry : virtual public Compound 
{
public:

  virtual ~LogEntry() {
  }

  virtual Table& joinLogEntryTable() = 0;
  virtual Value<Database::ID>& addId() = 0;
  virtual Interval<Database::DateTimeInterval>& addDateTime() = 0;
  virtual Value<std::string>& addSourceIp() = 0;
  virtual Value<Database::LogEventType>& addFlag() = 0;
  virtual Value<Database::LogComponent>& addComponent() = 0;
  virtual Value<std::string>& addContent() = 0;
  virtual Value<Database::ID>& addLogPropertyId() = 0;
  virtual LogProperty& addLogProperty() = 0;

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
  virtual Interval<Database::DateTimeInterval>& addDateTime();
  virtual Value<std::string>& addSourceIp();
  virtual Value<Database::LogEventType>& addFlag();
  virtual Value<Database::LogComponent>& addComponent();
  virtual Value<std::string>& addContent();
  virtual Value<Database::ID>& addLogPropertyId();
  virtual LogProperty& addLogProperty();

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

};

}

CONVERSION_DECLARATION(Database::LogEventType)

inline Database::LogEventType Conversion<Database::LogEventType>::from_string(const std::string& _value) {
  return (Database::LogEventType)atoi(_value.c_str());
}

inline std::string Conversion<Database::LogEventType>::to_string(const Database::LogEventType& _value) {
  return signed2string((int)_value);
}


CONVERSION_DECLARATION(Database::LogComponent)

inline Database::LogComponent Conversion<Database::LogComponent>::from_string(const std::string& _value) {
  return (Database::LogComponent)atoi(_value.c_str());
}

inline std::string Conversion<Database::LogComponent>::to_string(const Database::LogComponent& _value) {
  return signed2string((int)_value);
}

}

#endif
