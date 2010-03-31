#ifndef _REQUEST_IMPL_H_
#define _REQUEST_IMPL_H_
#include "request.h"

namespace Register {
namespace Logger {

class ManagerImpl : public Manager {
private:
    boost::mutex properties_mutex;

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

public:

  
  ManagerImpl(const std::string &monitoring_hosts_file = std::string()) throw(DB_CONNECT_FAILED);

  virtual ~ManagerImpl();

  /** Used only in migration  - return a connection used by the connection manager 
 	it's meant to be used only in single-threaded environment
  */
  Connection get_connection() {
	return Database::Manager::acquire();
  }

  Database::ID i_CreateRequest(const char *sourceIP, RequestServiceType service, const  char *content_in, const Register::Logger::RequestProperties& props, RequestActionType action_type, Database::ID session_id);
  bool i_UpdateRequest(Database::ID id, const Register::Logger::RequestProperties &props);
  bool i_CloseRequest(Database::ID id, const char *content_out, const Register::Logger::RequestProperties &props);
  bool i_CloseRequestLogin(Database::ID id, const char *content_out, const Register::Logger::RequestProperties &props, Database::ID session_id);
  Database::ID i_CreateSession(Languages lang, const char *name);
  bool i_CloseSession(Database::ID id);
  Database::Result i_GetServiceActions(RequestServiceType service);

 // for migration tool (util/logd_migration)
  void insert_props_pub(DateTime entry_time, RequestServiceType entry_service, bool monitoring, Database::ID entry_id, const Register::Logger::RequestProperties& props) {

	insert_props(entry_time, entry_service, monitoring, entry_id, props, get_connection());
  }
  
  List* createList() const;

private:
  bool close_request_worker(Connection &conn, ID id, const char *content_out, const Register::Logger::RequestProperties &props);
  void insert_props(DateTime entry_time, RequestServiceType service, bool monitoring, ID entry_id, const Register::Logger::RequestProperties& props, Connection conn);
  bool record_check(Database::ID id, Connection &conn);
  Database::ID find_property_name_id(const std::string &name, Connection &conn);
  inline Database::ID find_last_property_value_id(Connection &conn);
  inline Database::ID find_last_request_id(Connection &conn);
  inline std::string getSessionUserName(Connection &conn, Database::ID session_id);

public:
  static const std::string LAST_PROPERTY_VALUE_ID;
  static const std::string LAST_PROPERTY_NAME_ID;
  static const std::string LAST_ENTRY_ID;
  static const std::string LAST_SESSION_ID;
  static const int MAX_NAME_LENGTH;

};

}
}

#endif // _REQUEST_IMPL_H_

