#ifndef _LOG_IMPL_H_
#define _LOG_IMPL_H_

// database model -- should be one file for the whole server
#include "src/corba/log/db_settings.h"

#include <map>
#include "types/data_types.h"
#include "model/log_filter.h"
#include "register/request.h"

using namespace Database;

// some CORBA types redefined here
// to get rid of dependence on CORBA
// .... NO we can use database types instead

// Mapping of CORBA type
enum Languages { EN, CS /*, __max_Languages=0xffffffff */ };

class Impl_Log {
private:
    struct strCmp {
		bool operator()(const std::string &s1, const std::string &s2) const {
			return s1 < s2;
		}
	};
  /** Limit the number of entries read from request_property table
   * (which is supposed to contain limited number of distinct property names )
   */
  static const unsigned int PROP_NAMES_SIZE_LIMIT = 10000;

  /*
  std::tr1::unordered_map<std::string, Database::ID> property_names
  */
  std::map<std::string, Database::ID, strCmp> property_names;
  std::list<std::string> monitoring_ips;


// TODO just a test
  Register::Logger::RequestProperties ppp;

public:

  struct DB_CONNECT_FAILED { };

  Impl_Log(const std::string conn_db, const std::string &monitoring_hosts_file = std::string()) throw(DB_CONNECT_FAILED);

  virtual ~Impl_Log();

  /** Used only in migration  - return a connection used by the connection manager */
  Connection get_connection() {
	return Manager::acquire();
	// TODO is this safe?
  }

  Database::ID i_CreateRequest(const char *sourceIP, Database::RequestServiceType service, const  char *content_in, const Register::Logger::RequestProperties& props, Database::RequestActionType action_type, Database::ID session_id);
  bool i_UpdateRequest(Database::ID id, const Register::Logger::RequestProperties &props);
  bool i_CloseRequest(Database::ID id, const char *content_out, const Register::Logger::RequestProperties &props);
  bool i_CloseRequestLogin(Database::ID id, const char *content_out, const Register::Logger::RequestProperties &props, Database::ID session_id);
  Database::ID i_CreateSession(Languages lang, const char *name);
  bool i_CloseSession(Database::ID id);
  Database::Result i_GetServiceActions(Database::RequestServiceType service);

 // for migration tool (util/logd_migration)
  void insert_props_pub(DateTime entry_time, Database::RequestServiceType entry_service, bool monitoring, Database::ID entry_id, const Register::Logger::RequestProperties& props) {

	insert_props(entry_time, entry_service, monitoring, entry_id, props, get_connection());
  }

private:
  bool close_request_worker(Connection &conn, ID id, const char *content_out, const Register::Logger::RequestProperties &props);
  void insert_props(DateTime entry_time, Database::RequestServiceType service, bool monitoring, ID entry_id, const Register::Logger::RequestProperties& props, Connection conn);
  // TODO former version    void insert_props(DateTime entry_time, Database::ID entry_id, const Register::Logger::RequestProperties& props, Connection &conn);
  bool record_check(Database::ID id, Connection &conn);
  Database::ID find_property_name_id(const std::string &name, Connection &conn);
  inline Database::ID find_last_property_value_id(Connection &conn);
  inline Database::ID find_last_request_id(Connection &conn);

public:
  static const std::string LAST_PROPERTY_VALUE_ID;
  static const std::string LAST_PROPERTY_NAME_ID;
  static const std::string LAST_ENTRY_ID;
  static const std::string LAST_SESSION_ID;
  static const int MAX_NAME_LENGTH;

};

#endif

