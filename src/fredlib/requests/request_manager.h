/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef REQUEST_MANAGER_H_
#define REQUEST_MANAGER_H_

#include "request_cache.h"
#include "session_cache.h"
#include "request_property_name_cache.h"

// FRED logging
#include "log/logger.h"
#include "log/context.h"

#include <stdexcept>

#include "random.h"

namespace Fred {
namespace Logger {

#ifdef HAVE_LOGGER
class logd_ctx_init {
public:
    inline logd_ctx_init() :
        ctx( (boost::format("logd-<%1%>") % Random::integer(0, 100000000)).str() )
    {}

private:
    Logging::Context ctx;
};

#else

class logd_ctx_init { };

#endif


typedef long int ServiceType;
typedef long int RequestType;

class List;

class InternalServerError : public std::runtime_error {
public:
    InternalServerError(const std::string msg) : std::runtime_error(msg) {};
};

class WrongUsageError : public std::runtime_error {
public:
    WrongUsageError(const std::string msg) : std::runtime_error(msg) {};
};

typedef std::map<std::string, unsigned long long>  RequestCountInfo;

class Manager {
public:

  struct DB_CONNECT_FAILED : public std::runtime_error
  {
      DB_CONNECT_FAILED()
              : std::runtime_error("Database connection failed")
      {}
  };

  virtual ~Manager() {};

  /** Used only in migration  - return a connection used by the connection manager
    it's meant to be used only in single-threaded environment
  */
virtual  Database::ID i_createRequest(const char *sourceIP, ServiceType service, const  char *content, const Fred::Logger::RequestProperties& props, const Fred::Logger::ObjectReferences &refs, RequestType request_type_id, Database::ID session_id) = 0;
virtual  bool i_addRequestProperties(Database::ID id, const Fred::Logger::RequestProperties &props) = 0;
virtual  bool i_closeRequest(Database::ID id, const char *content, const Fred::Logger::RequestProperties &props, const Fred::Logger::ObjectReferences &refs, const long result_code, Database::ID session_id) = 0;
  virtual Database::ID i_createSession(Database::ID id, const char *name) = 0;
virtual  bool i_closeSession(Database::ID id) = 0;
  virtual Database::Result i_getRequestTypesByService(ServiceType service) = 0;
  virtual Database::Result i_getServices() = 0;
  virtual Database::Result i_getResultCodesByService(ServiceType service) = 0;
  virtual Database::Result i_getObjectTypes() = 0;
  virtual unsigned long long i_getRequestCount(const boost::posix_time::ptime &datetime_from, const boost::posix_time::ptime &datetime_to, const std::string &service, const std::string &user) = 0;
  virtual std::auto_ptr<RequestCountInfo> i_getRequestCountUsers(const boost::posix_time::ptime &datetime_from, const boost::posix_time::ptime &datetime_to, const std::string &service) = 0;

  virtual List* createList() const = 0;

  static Manager *create();
  static Manager *create(const std::string conn_db, const std::string &monitoring_hosts_file = std::string());

};

class ManagerImpl : public Manager {
private:

  std::list<std::string> monitoring_ips;
  RequestCache rcache;
  SessionCache scache;
  std::auto_ptr<RequestPropertyNameCache> pcache;

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
  // timestamps in local time
  unsigned long long i_getRequestCount(const boost::posix_time::ptime &datetime_from, const boost::posix_time::ptime &datetime_to, const std::string &service, const std::string &user);
  std::auto_ptr<RequestCountInfo> i_getRequestCountUsers(const boost::posix_time::ptime &datetime_from, const boost::posix_time::ptime &datetime_to, const std::string &service);

  // for migration tool (util/logd_migration)
  void insert_props_pub(DateTime entry_time, ServiceType request_service_id, bool monitoring, Database::ID request_id, const Fred::Logger::RequestProperties& props);

  List* createList() const;

private:

  void insert_props(DateTime entry_time, ServiceType service, bool monitoring, ID request_id, const Fred::Logger::RequestProperties& props, Connection &conn, bool output);
  void insert_obj_ref(DateTime entry_time, ServiceType service, bool monitoring, ID request_id, const Fred::Logger::ObjectReferences& props, Connection &conn);
  bool record_check(Database::ID id, Connection &conn);

  inline Database::ID find_last_property_value_id(Connection &conn);
  inline Database::ID find_last_request_id(Connection &conn);
  inline void getSessionUser(Connection &conn, Database::ID session_id, std::string *user_name, Database::ID *user_id);

  Result getRequestCountUsersWorker(ptime from, ptime to, int service_id);
  unsigned long long getRequestCountWorker(ptime from, ptime to, int service_id, std::string user_name);
  void incrementRequestCounts(RequestCountInfo *inf_ptr, Result res);
};

}
}

#endif /* REQUEST_MANAGER_H_ */
