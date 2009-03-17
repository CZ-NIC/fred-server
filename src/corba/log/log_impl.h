#ifndef _LOG_IMPL_H_

#define _LOG_IMPL_H_


#include <boost/noncopyable.hpp>
#include "register/db_settings.h"
/* TODO : to remove  */

#include <map>


#include "types/data_types.h"

using namespace Database;


// some CORBA types redefined here
// to get rid of dependence on CORBA
// .... NO we can use database types instead

struct LogProperty : public boost::noncopyable {
  std::string name;
  std::string value;
  bool output;
  bool child;
};

class LogProperties : public boost::noncopyable {
  int size;
  LogProperty *buf;

public:
  LogProperties() : size(0) {
	  buf = NULL;
  }

  LogProperties(int init_size) : size(init_size) {
	if(init_size > 0) {
		buf = new LogProperty[init_size];
	} else {
		buf = NULL;
	}
  }

  virtual ~LogProperties() {
	if (buf != NULL) delete [] buf;
  }

  LogProperty & operator[] (size_t idx) {
	if (buf==NULL)   throw std::logic_error("No buffer allocated");
	if (idx >= size) throw std::logic_error("Index out of bounds");
	return buf[idx];
  }

  const LogProperty & operator[] (size_t idx) const throw (std::logic_error) {
	if (buf==NULL)   throw std::logic_error("No buffer allocated");
	if (idx >= size) throw std::logic_error("Index out of bounds");
	return buf[idx];
  }

  size_t length() const {
	return size;
  }

  void length(size_t l) {
	size = l;
  }
  // TODO more methods
};

// TODO move to Database::
enum Languages { EN, CS /*, __max_Languages=0xffffffff */ };

// TODO this is duplicity (src/model/log_filter.h):
enum LogServiceType { LC_UNIX_WHOIS, LC_WEB_WHOIS, LC_PUBLIC_REQUEST, LC_EPP, LC_WEBADMIN };

// END of CORBA types,  ------------------------------------
// for the rest, Database:: types are used

class Impl_Log {
private:
    struct strCmp {
		bool operator()(const std::string &s1, const std::string &s2) const {
			return s1 < s2;
		}
	};
  /** Limit the number of entries read from log_property_name table
   * (which is supposed to contain limited number of distinct property names )
   */
  static const int PROP_NAMES_SIZE_LIMIT = 10000;

  Manager db_manager;

  /*
  std::tr1::unordered_map<std::string, Database::ID> property_names
  */
  std::map<std::string, Database::ID, strCmp> property_names;

public:

  struct DB_CONNECT_FAILED { };

  Impl_Log(const std::string conn_db) throw(DB_CONNECT_FAILED);
  virtual ~Impl_Log();



  Database::ID i_new_event(const char *sourceIP, LogServiceType service, const char *content_in, const LogProperties& props);
  bool i_update_event(Database::ID id, const LogProperties &props);
  bool i_update_event_close(Database::ID id, const char *content_out, const LogProperties &props);
  Database::ID i_new_session(Languages lang, const char *name, const char *clTRID);
  bool i_end_session(Database::ID id, const char *clTRID);

  // used by unittest
  inline Database::ID find_last_log_entry_id(Connection &conn);

private:
  void insert_props(Database::ID entry_id, const LogProperties& props, Connection &conn);
  bool record_check(Database::ID id, Connection &conn);
  Database::ID find_property_name_id(const std::string &name, Connection &conn);
  inline Database::ID find_last_property_value_id(Connection &conn);

  static const std::string LAST_PROPERTY_VALUE_ID;
  static const std::string LAST_PROPERTY_NAME_ID;
  static const std::string LAST_ENTRY_ID;
  static const std::string LAST_SESSION_ID;

};

#endif
