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
 *  Manager_ for database connections.
 */


#ifndef DATABASE_MANAGER_H_
#define DATABASE_MANAGER_H_

#include <string>

#include "result.h"
#include "connection.h"
#include "transaction.h"
#include "sequence.h"

#include "config.h"

#ifdef HAVE_LOGGER
#include "log/logger.h"
#endif

namespace Database {

/**
 * \class Manager_
 * \brief Simple database manager
 *
 * This object populated with specific connection factory can be used as 
 * connection manager
 */
template<class connection_factory>
class Manager_ {
public:
  typedef typename connection_factory::connection_driver                        connection_driver;
  typedef Connection_<connection_driver, Manager_>                              connection_type;
  typedef Transaction_<typename connection_driver::transaction_type, Manager_>  transaction_type;
  typedef Result_<typename connection_driver::result_type>                      result_type;
  typedef Sequence_<connection_type, Manager_>                                  sequence_type;
  typedef typename result_type::Row                                             row_type;

  /**
   * Constuctors and destructor
   */
  Manager_(connection_factory *_conn_factory) : conn_factory_(_conn_factory) {
  }

  virtual ~Manager_() {
    if (conn_factory_) {
      delete conn_factory_;
    }
  }


  /**
   * Connection factory methods
   *
   * @return  connection pointer
   *          TODO: this is for bacward compatibility with fred server
   *                it should return automanaged connection
   */
  virtual connection_type* getConnection() {
    return acquire();
  }

  virtual const std::string& getConnectionString() {
    return conn_factory_->getConnectionString();
  }   
  
  virtual connection_type* acquire() {
    /* get the connection string from configured factory to support
     * lazy connection opening 
     *
     * TODO: use connection factory acquire() method and pass manager pointer
     *       for specific factory destruct callback
     * */
    return new connection_type(conn_factory_->getConnectionString()); 
  }


protected:
  connection_factory *conn_factory_;
};


}


#endif /*DATABASE_MANAGER_H_*/

