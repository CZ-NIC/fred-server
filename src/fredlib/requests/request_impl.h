#ifndef _REQUEST_IMPL_H_
#define _REQUEST_IMPL_H_
#include "request.h"
#include "request_cache.h"

namespace Fred {
namespace Logger {

class InternalServerError : std::runtime_error {
public:
    InternalServerError(const std::string msg) : std::runtime_error(msg) {};
};

class WrongUsageError : std::runtime_error {
public:
    WrongUsageError(const std::string msg) : std::runtime_error(msg) {};
};

class ManagerImpl : public Manager {
private:
    boost::mutex properties_mutex;

    struct strCmp {
		bool operator()(const std::string &s1, const std::string &s2) const {
			return s1 < s2;
		}
	};
  /** Limit the number of entries read from request_property_name table
   * (which is supposed to contain limited number of distinct property names )
   */
  static const unsigned int PROP_NAMES_SIZE_LIMIT = 10000;

  /*
  std::tr1::unordered_map<std::string, Database::ID> property_names
  */
  std::map<std::string, Database::ID, strCmp> property_names;
  std::list<std::string> monitoring_ips;
  RequestCache rcache;

public:

  
  ManagerImpl(const std::string &monitoring_hosts_file = std::string());

  virtual ~ManagerImpl();

  /** Used only in migration  - return a connection used by the connection manager 
 	it's meant to be used only in single-threaded environment
  */
  Connection get_connection() {
	return Database::Manager::acquire();
  }

  Database::ID i_createRequest(const char *sourceIP, ServiceType service, const  char *content, const Fred::Logger::RequestProperties& props, const Fred::Logger::ObjectReferences &refs, RequestType request_type_id, Database::ID session_id);
  bool i_addRequestProperties(Database::ID id, const Fred::Logger::RequestProperties &props);
  bool i_closeRequest(Database::ID id, const char *content, const Fred::Logger::RequestProperties &props, const Fred::Logger::ObjectReferences &refs, const long result_code, Database::ID session_id);
  Database::ID i_createSession(Database::ID id, const char *name);
  bool i_closeSession(Database::ID id);
  Database::Result i_getRequestTypesByService(ServiceType service);
  Database::Result i_getServices();
  Database::Result i_getResultCodesByService(ServiceType service);
  Database::Result i_getObjectTypes();

 // for migration tool (util/logd_migration)
 void insert_props_pub(DateTime entry_time, ServiceType request_service_id, bool monitoring, Database::ID request_id, const Fred::Logger::RequestProperties& props);
  
  List* createList() const;

private:
  
  void insert_props(DateTime entry_time, ServiceType service, bool monitoring, ID request_id, const Fred::Logger::RequestProperties& props, Connection &conn, bool output);
  void insert_obj_ref(DateTime entry_time, ServiceType service, bool monitoring, ID request_id, const Fred::Logger::ObjectReferences& props, Connection &conn);
  bool record_check(Database::ID id, Connection &conn);
  Database::ID find_property_name_id(const std::string &name, Connection &conn);
  inline Database::ID find_last_property_value_id(Connection &conn);
  inline Database::ID find_last_request_id(Connection &conn);
  inline void getSessionUser(Connection &conn, Database::ID session_id, std::string *user_name, Database::ID *user_id);

public:
  static const int MAX_NAME_LENGTH;

};

}
}

#endif // _REQUEST_IMPL_H_

