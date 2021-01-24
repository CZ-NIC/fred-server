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
#include <pthread.h>
#include <boost/algorithm/string.hpp>
#include <utility>

#include "config.h"


#include "src/deprecated/libfred/requests/request.hh"
#include "src/deprecated/libfred/requests/request_manager.hh"
#include "src/deprecated/libfred/requests/request_list.hh"

#include "src/deprecated/libfred/requests/model_session.hh"
#include "src/deprecated/libfred/requests/model_request.hh"


namespace LibFred {
namespace Logger {

using namespace Database;

using namespace boost::posix_time;
using namespace boost::gregorian;

/**
         *  Database access class designed for use in fred-logd with partitioned tables
         *  It ensures that the connection will have constraint_exclusion enabled
         *  and all queries will be performed in one transaction which
         *  retains the features of Database::Transaction - it has to be commited
         *  explicitly by the client, otherwise it's rollbacked.
         *  The class acquires database connection from the Database::Manager and explicitly
         *  releases in destructor.
         *  DEPENDENCIES:
         *   - It assumes that the system error logger is initialized (use of logger_error() function
         *   - there's only one connection per thread, so the DB framework receives
         *     the connection encapsulated here
         */
// ManagerImpl ctor: connect to the database and fill property_names map
ManagerImpl::ManagerImpl()
{
        std::ifstream file;

    logd_ctx_init ctx;

        Connection conn = Database::Manager::acquire();

    logger_notice("Logger startup - successfully connected to DATABASE ");
}

ManagerImpl::~ManagerImpl() {
  logd_ctx_init ctx;

  logger_notice("Logging destructor");
}

Manager* Manager::create() {
    TRACE("[CALL] LibFred::Logger::Manager::create()");
    return new ManagerImpl();
}

} // namespace LibFred::Logger
} // namespace LibFred
