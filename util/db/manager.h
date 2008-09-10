/*  
 * Copyright (C) 2007  CZ.NIC, z.s.p.o.
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

/**
 *  @file manager.h
 *  Manager for database connections.
 */


#ifndef DB_MANAGER_H_
#define DB_MANAGER_H_

#include <string>
#include "database.h"
#include "sequence.h"

#include "config.h"

#ifdef HAVE_LOGGER
#include "log/logger.h"
#endif

namespace Database {

/**
 * \class Manager
 * \brief Base class representing database manager
 *
 * Manager object (which should be used) is defined in \file database.h
 * file.
 *
 * This class can be subclassed to more specific Manager behaviour
 * i.e. for support connection pooling
 */
class Manager {
public:
  typedef Connection connection_type;

  /**
   * Constuctors and destructor
   */
	Manager(const std::string& _conn_info) : conn_info_(_conn_info) {
#ifdef HAVE_LOGGER
    TRACE(boost::format("<CALL> Database::Manager::Manager('%1%')") % conn_info_);
#endif
	}


	Manager(const char* _conn_info) :	conn_info_(_conn_info) {
	}


	virtual ~Manager() {
#ifdef HAVE_LOGGER
    TRACE("<CALL> Database::Manager::~Manager()");
#endif
	}


	virtual connection_type* getConnection() {
    return new connection_type(conn_info_);
  }

protected:
	std::string conn_info_;
};


}
#endif /*DB_MANAGER_H_*/
