/*
 * Copyright (C) 2011-2020  CZ.NIC, z. s. p. o.
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

#include "src/deprecated/libfred/requests/request_property_name_cache.hh"

// FRED logging
#include "util/log/logger.hh"
#include "util/log/context.hh"

#include <stdexcept>

#include "util/random/random.hh"

#include "src/deprecated/libfred/db_settings.hh"

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

  virtual List* createList() const = 0;

  static Manager *create();
  static Manager *create(const std::string conn_db);

};

class ManagerImpl : public Manager {
public:


  ManagerImpl();

  virtual ~ManagerImpl();

  /** Used only in migration  - return a connection used by the connection manager
    it's meant to be used only in single-threaded environment
  */
  Connection get_connection() {
    return Database::Manager::acquire();
  }

  List* createList() const;
};

}
}

#endif /* REQUEST_MANAGER_H_ */
