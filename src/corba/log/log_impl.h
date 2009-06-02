#ifndef _LOG_IMPL_H_
#define _LOG_IMPL_H_

// database model
#include "db_settings.h"
/*
#include "model_log_action_type.h"
#include "model_log_entry.h"
#include "model_log_property_name.h"
#include "model_log_property_value.h"
#include "model_log_raw_content.h"
#include "model_log_session.h"
*/


//   #include "register/db_settings.h"
/* TODO : to remove  */

#include <map>
#include "types/data_types.h"

using namespace Database;

// some CORBA types redefined here
// to get rid of dependence on CORBA
// .... NO we can use database types instead

struct LogProperty {
  std::string name;
  std::string value;
  bool output;
  bool child;

  LogProperty() : name(), value() {
  }

  LogProperty(const LogProperty &p) : name(p.name), value(p.value), 
				output(p.output), child(p.child) {

  }
  const LogProperty & operator = (LogProperty &p) {
	name = p.name;
	value = p.value;
	output = p.output;
	child = p.child;

	return *this;
  }
};


typedef std::vector<LogProperty> LogProperties;

/*
class LogProperties : public boost::noncopyable {
  size_t alloc_size, size;
  LogProperty *buf;

public:
  LogProperties() : alloc_size(0), size(0) {
	  buf = NULL;
  }

  LogProperties(int init_size) : alloc_size(init_size), size(0) {
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
	if (idx >= alloc_size) throw std::logic_error("Index out of bounds");
	return buf[idx];
  }

  const LogProperty & operator[] (size_t idx) const throw (std::logic_error) {
	if (buf==NULL)   throw std::logic_error("No buffer allocated");
	if (idx >= alloc_size) throw std::logic_error("Index out of bounds");
	return buf[idx];
  }

  size_t maximum() const {
	return alloc_size;
  }

  void maximum(size_t l) {
	alloc_size = l;
  }
  // TODO more methods
};
*/

// TODO move to Database::
enum Languages { EN, CS /*, __max_Languages=0xffffffff */ };

// TODO this is duplicity (src/model/log_filter.h):
enum LogServiceType { LC_UNIX_WHOIS, LC_WEB_WHOIS, LC_PUBLIC_REQUEST, LC_EPP, LC_WEBADMIN };

// END of CORBA types,  ------------------------------------
// for the rest, Database:: types are used

class MyManager {
public:
	MyManager(ConnectionFactory *fact) {};

	Connection *acquire() {	
		return new Connection(Manager::acquire());
	}

};

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
  static const unsigned int PROP_NAMES_SIZE_LIMIT = 10000;



  /*
  std::tr1::unordered_map<std::string, Database::ID> property_names
  */
  std::map<std::string, Database::ID, strCmp> property_names;
  std::list<std::string> monitoring_ips;

public:

  struct DB_CONNECT_FAILED { };



  Impl_Log(const std::string conn_db, const std::string &monitoring_hosts_file = std::string()) throw(DB_CONNECT_FAILED);

  virtual ~Impl_Log();

  /** Used only in migration  - return a connection used by the connection manager */
  Connection get_connection() {
	return Manager::acquire();
	// TODO is this safe?
  }

  Database::ID i_new_event(const char *sourceIP, LogServiceType service, const  char *content_in, const LogProperties& props, int action_type);
  bool i_update_event(Database::ID id, const LogProperties &props);
  bool i_update_event_close(Database::ID id, const char *content_out, const LogProperties &props);
  Database::ID i_new_session(Languages lang, const char *name, const char *clTRID);
  bool i_end_session(Database::ID id, const char *clTRID);

 // for migration tool (util/logd_migration)
  void insert_props_pub(DateTime entry_time, Database::ID entry_id, const LogProperties& props) {
	insert_props(entry_time, entry_id, props, get_connection());
  }

private:
  void insert_props(DateTime entry_time, Database::ID entry_id, const LogProperties& props, Connection conn);
  // TODO former version    void insert_props(DateTime entry_time, Database::ID entry_id, const LogProperties& props, Connection &conn);
  bool record_check(Database::ID id, Connection &conn);
  Database::ID find_property_name_id(const std::string &name, Connection &conn);
  inline Database::ID find_last_property_value_id(Connection &conn);
  inline Database::ID find_last_log_entry_id(Connection &conn);

public:
  static const std::string LAST_PROPERTY_VALUE_ID;
  static const std::string LAST_PROPERTY_NAME_ID;
  static const std::string LAST_ENTRY_ID;
  static const std::string LAST_SESSION_ID;
  static const int MAX_NAME_LENGTH;

};

#endif

