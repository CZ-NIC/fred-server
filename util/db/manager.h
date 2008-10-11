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
#include <queue>
#include <map>
#include <algorithm>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>


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


  Manager(const char* _conn_info) : conn_info_(_conn_info) {
  }


  virtual ~Manager() {
#ifdef HAVE_LOGGER
    TRACE("<CALL> Database::Manager::~Manager()");
#endif
  }


  /**
   * @return  connection
   */
  virtual connection_type* getConnection() {
    return new connection_type(conn_info_);
  }

protected:
  std::string conn_info_; /**< connection string (host, database name, user, password ...) */
};



/**
 * \class Pool
 * \brief Pool of connections
 *
 * Specialized database manager implementing connection pooling
 */
class ConnectionPool : public Manager {
protected:
  /**
   * internal data structure for storing information about connection
   */
  struct conn_data_ {
    conn_data_(unsigned _id, bool _used) :
               id(_id), used(_used) {
    }

    unsigned         id;     /**< connection id */
    bool             used;   /**< flag if connection is used or not */
  };

  typedef std::map<connection_type*, conn_data_> storage_type;
  typedef std::queue<connection_type*>           queue_type;

  unsigned     init_conn_;   /**< connections established at init */
  unsigned     max_conn_;    /**< maximum number of connection */

  storage_type             pool_;        /**< connection pool (conn and info data) */
  queue_type               free_;        /**< free conns for quick acces */
  mutable boost::mutex     pool_lock_;   /**< thread safety */
  mutable boost::condition has_free_;    /**< wait condition when no connection available */


public:
  /**
   * constuctor and destructor
   */
  ConnectionPool(const std::string &_conn_info,
       unsigned _init_conn = 0,
       unsigned _max_conn = 1) : Manager(_conn_info),
                                 init_conn_(_init_conn),
				 max_conn_(_max_conn) {
    relax_(init_conn_);
  }


  virtual ~ConnectionPool() {
    storage_type::iterator it = pool_.begin();
    for (; it != pool_.end(); ++it) {
      delete it->first;
      pool_.erase(it);
    }
  }


  /**
   * get connection from pool
   *
   * @return  connection
   */
  virtual connection_type* acquire() {
    boost::mutex::scoped_lock scoped_lock(pool_lock_);

    /**
     * if no connection is free and pool has enough capacity
     * we will relax pool size
     */
    if (!free_.size() && pool_.size() < max_conn_) {
      relax_(pool_.size() + std::min<unsigned>((unsigned)5, max_conn_ - pool_.size()));
    }

    while (!free_.size()) {
#ifdef HAVE_LOGGER
      LOGGER(PACKAGE).debug("waiting for free connection...");
#endif
      has_free_.wait(scoped_lock);
    }

    if (free_.size()) {
      connection_type *conn = free_.front();
      free_.pop();

      storage_type::iterator it = pool_.find(conn);
      if (it != pool_.end()) {
        it->second.used = true;
#ifdef HAVE_LOGGER
        LOGGER(PACKAGE).debug(boost::format("acquired connection id=%1%") % it->second.id);
        logStatus_();
#endif
        return it->first;
      }
      else {
        /**
         * free connection queue and storage map is not synchronized?!
         * should not happen!
         */
        throw Exception("Oops! Bug in connection pool management!");
      }
    }
    else {
      /**
       * no connection available 
       * should not happend - when connection is not available
       * thread is sleeped until another released one and notify it
       */
      throw Exception("No free connection available!");
    }
    
  }


  /**
   * overriden method from Database::Manager for getting connection
   * see above
   */
  virtual connection_type* getConnection() {
    return acquire();
  }


  /**
   * release connection back to pool; there can't be any transaction
   * active on the connection!
   *
   * @param _conn  connection acquired from this pool
   */
  virtual void release(connection_type *_conn) {
    boost::mutex::scoped_lock scoped_lock(pool_lock_);

    /* find connection data */
    storage_type::iterator it = pool_.find(_conn);
    if (it == pool_.end()) {
      /**
       * connection is not in the pool
       * should not happen!
       */
#ifdef HAVE_LOGGER
      LOGGER(PACKAGE).warning("connection to release is not in the pool!");
#endif
      return;
    }

    if (it->second.used == false) {
      /**
       * Trying to release not used connection
       * should not happen!
       */
#ifdef HAVE_LOGGER
      LOGGER(PACKAGE).warning(boost::format("try to release unused connection id=%1%!?") % it->second.id);
#endif
      return;
    }
    else {
      /* flag and reset released connection */
      it->second.used = false;
      _conn->reset();
      free_.push(_conn);
#ifdef HAVE_LOGGER
      LOGGER(PACKAGE).debug(boost::format("released connection id=%1%") % it->second.id);
      logStatus_();
#endif
      has_free_.notify_one();
    }
  }

  
  /**
   * thread safe usage of internal method
   *
   * @return  string representation of 
   *          actual state of pool (size, used, free)
   */
  virtual std::string status() const {
    boost::mutex::scoped_lock scoped_lock(pool_lock_);
    std::string ret = status_();
    return ret;
  }


private:
  /**
   * internal method for relaxing pool size up and down
   * for downsize there should be enough of free connections
   * for upsize actual pool size should be lesser then maximum connection allowed
   *
   * should be called when pool lock is acquired!
   *
   * @param  _to  design number of connection in pool
   */
  virtual void relax_(unsigned _to) {
#ifdef HAVE_LOGGER
      LOGGER(PACKAGE).debug(boost::format("relaxing pool size %1% --> %2%") 
                                          % pool_.size()
                                          % _to);
#endif
    if (_to < pool_.size()) {
      /* TODO: */
#ifdef HAVE_LOGGER
      if (_to != pool_.size()) {
        LOGGER(PACKAGE).warning("can't relax pool size down - not enough free connections");
      }
#endif
      return;
    }

    for (unsigned i = pool_.size(); i < _to; ++i) {
      /* create new connection and its info data */
      connection_type *conn = new connection_type(conn_info_, false);
      conn_data_ cd(i, false);
      /* store it in pool and enqueue into free */
      pool_.insert(std::make_pair<connection_type*, conn_data_>(conn, cd));
      free_.push(conn);
#ifdef HAVE_LOGGER
      LOGGER(PACKAGE).debug(boost::format("added new connection id=%1%") % i);
#endif
    }
  }


  /**
   * internal method
   * should be called when pool lock is acquired!
   * 
   * @param  _to  design number of connection in pool
   */
  std::string status_() const {
    unsigned total = pool_.size();
    unsigned used = 0;
    unsigned free = 0;

    storage_type::const_iterator it = pool_.begin();
    for (; it != pool_.end(); ++it) {
      if (it->second.used == true)
        used += 1;
      else
        free += 1;
    }

    return str(boost::format("connection pool summary: "
                             "total=%1% (used=%2% free=%3%)")
                             % total % used % free);
  }

#ifdef HAVE_LOGGER
  inline void logStatus_() const {
    LOGGER(PACKAGE).info(status_());
  }
#endif
};

}
#endif /*DB_MANAGER_H_*/
