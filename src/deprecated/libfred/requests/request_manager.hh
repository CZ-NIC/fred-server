/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
#ifndef REQUEST_MANAGER_HH_F4CF50B792BC4330B21A6EB0D5A4546F
#define REQUEST_MANAGER_HH_F4CF50B792BC4330B21A6EB0D5A4546F

#include "src/deprecated/libfred/requests/request_cache.hh"
#include "src/deprecated/libfred/requests/session_cache.hh"
#include "src/deprecated/libfred/requests/request_property_name_cache.hh"

// FRED logging
#include "util/log/logger.hh"
#include "util/log/context.hh"

#include <stdexcept>

#include "util/random/random.hh"

namespace LibFred {
namespace Logger {

#ifdef HAVE_LOGGER
class logd_ctx_init {
public:
    inline logd_ctx_init() :
        ctx( (boost::format("logd-<%1%>") % Random::Generator().get(0, 100000000)).str() )
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
virtual  Database::ID i_createRequest(const char *sourceIP, ServiceType service, const  char *content, const LibFred::Logger::RequestProperties& props, const LibFred::Logger::ObjectReferences &refs, RequestType request_type_id, Database::ID session_id) = 0;
virtual  bool i_closeRequest(Database::ID id, const char *content, const LibFred::Logger::RequestProperties &props, const LibFred::Logger::ObjectReferences &refs, const long result_code, Database::ID session_id) = 0;
  virtual Database::ID i_createSession(Database::ID id, const char *name) = 0;
virtual  bool i_closeSession(Database::ID id) = 0;
  virtual Database::Result i_getRequestTypesByService(ServiceType service) = 0;
  virtual Database::Result i_getServices() = 0;
  virtual Database::Result i_getResultCodesByService(ServiceType service) = 0;
  virtual Database::Result i_getObjectTypes() = 0;
  virtual unsigned long long i_getRequestCount(const boost::posix_time::ptime &datetime_from, const boost::posix_time::ptime &datetime_to, const std::string &service, const std::string &user) = 0;
  virtual std::unique_ptr<RequestCountInfo> i_getRequestCountUsers(const boost::posix_time::ptime &datetime_from, const boost::posix_time::ptime &datetime_to, const std::string &service) = 0;

  virtual List* createList() const = 0;

  static Manager *create();
  static Manager *create(const std::string conn_db, const std::string &monitoring_hosts_file = std::string());

};

class ManagerImpl : public Manager {
private:

  std::list<std::string> monitoring_ips;
  RequestCache rcache;
  SessionCache scache;
  std::unique_ptr<RequestPropertyNameCache> pcache;

public:


  ManagerImpl(const std::string &monitoring_hosts_file = std::string());

  virtual ~ManagerImpl();

  /** Used only in migration  - return a connection used by the connection manager
    it's meant to be used only in single-threaded environment
  */
  Connection get_connection() {
    return Database::Manager::acquire();
  }

  Database::ID i_createRequest(const char *sourceIP, ServiceType service, const  char *content, const LibFred::Logger::RequestProperties& props, const LibFred::Logger::ObjectReferences &refs, RequestType request_type_id, Database::ID session_id);
  bool i_closeRequest(Database::ID id, const char *content, const LibFred::Logger::RequestProperties &props, const LibFred::Logger::ObjectReferences &refs, const long result_code, Database::ID session_id);
  Database::ID i_createSession(Database::ID id, const char *name);
  bool i_closeSession(Database::ID id);
  Database::Result i_getRequestTypesByService(ServiceType service);
  Database::Result i_getServices();
  Database::Result i_getResultCodesByService(ServiceType service);
  Database::Result i_getObjectTypes();
  // timestamps in local time
  unsigned long long i_getRequestCount(const boost::posix_time::ptime &datetime_from, const boost::posix_time::ptime &datetime_to, const std::string &service, const std::string &user);
  std::unique_ptr<RequestCountInfo> i_getRequestCountUsers(const boost::posix_time::ptime &datetime_from, const boost::posix_time::ptime &datetime_to, const std::string &service);

  // for migration tool (util/logd_migration)
  void insert_props_pub(Database::DateTime entry_time, ServiceType request_service_id, bool monitoring, Database::ID request_id, const LibFred::Logger::RequestProperties& props);

  List* createList() const;

private:

  void insert_props(Database::DateTime entry_time, ServiceType service, bool monitoring, ID request_id, const LibFred::Logger::RequestProperties& props, Connection &conn, bool output);
  void insert_obj_ref(Database::DateTime entry_time, ServiceType service, bool monitoring, ID request_id, const LibFred::Logger::ObjectReferences& props, Connection &conn);
  bool record_check(Database::ID id, Connection &conn);

  inline Database::ID find_last_property_value_id(Connection &conn);
  inline Database::ID find_last_request_id(Connection &conn);
  inline void getSessionUser(Connection &conn, Database::ID session_id, std::string *user_name, Database::ID *user_id);

  Result getRequestCountUsersWorker(ptime from, ptime to, int service_id);
  unsigned long long getRequestCountWorker(ptime from, ptime to, int service_id, std::string user_name);
  void incrementRequestCounts(RequestCountInfo *inf_ptr, Result res);
};

  unsigned long long insert_property_record_impl(
      Database::Connection &_conn,
      const Database::DateTime &_request_time,
      ServiceType _service,
      bool _monitoring,
      unsigned long long _request_id,
      unsigned long long _property_name_id,
      const std::string &_value,
      bool _output,
      unsigned long long _parent_id
  );


}
}

#endif /* REQUEST_MANAGER_H_ */
