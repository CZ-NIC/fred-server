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
 *  @file connection_factory.h
 *  
 *  Factories defines how connection is acquired and released.
 *  Definition of simple factory and connection pool factory follows.
 */


#ifndef CONNECTION_FACTORY_H_
#define CONNECTION_FACTORY_H_


#include <string>
#include <queue>
#include <map>
#include <algorithm>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/format.hpp>

#include "result.h"
#include "config.h"

#ifdef HAVE_LOGGER
#include "log/logger.h"
#endif


namespace Database {
namespace Factory {

/**
 * \class  Simple
 * \brief  Very simple factory using constructor and destructor of
 *         connection_driver type
 */
template<class conndriver>
class Simple {
private:
  std::string conn_info_; /**< connection string (host, database name, user, password ...) */

public:
  typedef conndriver  connection_driver;

  /**
   * Constuctors and destructor
   */
  Simple(const std::string& _conn_info) : conn_info_(_conn_info) {
#ifdef HAVE_LOGGER
    TRACE(boost::format("<CALL> Database::Factory::Simple::Simple('%1%')") % conn_info_);
#endif
  }


  Simple(const char* _conn_info) : conn_info_(_conn_info) {
  }


  virtual ~Simple() {
#ifdef HAVE_LOGGER
    TRACE("<CALL> Database::Factory::Simple::~Simple()");
#endif
  }


  /**
   * Connection factory method
   *
   * @return  connection
   */
  virtual inline connection_driver* acquire() {
    return new connection_driver(conn_info_);
  }


  /**
   * Simple connection releaser - delete it
   *
   * @param _conn  connection pointer
   */
  virtual void release(connection_driver *&_conn) {
    if (_conn) {
      delete _conn;
      _conn = 0;
    }
  }


  /**
   * Connection string getter
   *
   * @return  connection string factory was configured with
   */
  virtual inline const std::string& getConnectionString() const {
    return conn_info_;
  }
};



/**
 * \class  ConnectionPool
 * \brief  Connection factory implementing pool of connections
 */
template<class conndriver>
class ConnectionPool {
public:
  typedef conndriver  connection_driver;

  /**
   * constuctor and destructor
   */
  ConnectionPool(const std::string &_conn_info,
                 unsigned _init_conn = 0,
                 unsigned _max_conn = 1) 
               : conn_info_(_conn_info),
                 init_conn_((_init_conn < _max_conn) ? _init_conn : _max_conn),
                 max_conn_(_max_conn) {
#ifdef HAVE_LOGGER
    TRACE("Database::Factory::ConnectionPool::ConnectionPool()");
    LOGGER(PACKAGE).info(boost::format("connection pool configured; conn_info='%1%' (init=%2% max=%3%)")
                                      % conn_info_ % init_conn_ % max_conn_);
#endif
    relax_(init_conn_);
  }


  virtual ~ConnectionPool() {
#ifdef HAVE_LOGGER
    TRACE("Database::Factory::ConnectionPool::~ConnectionPool()");
#endif
    boost::mutex::scoped_lock scoped_lock(pool_lock_);

    typename storage_type::iterator it = pool_.begin();
    for (; it != pool_.end(); ++it) {
      delete it->first;
    }
  }


  /**
   * get connection from pool
   *
   * @return  connection
   */
  virtual connection_driver* acquire() {
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
      connection_driver *conn = free_.front();
      free_.pop();

      typename storage_type::iterator it = pool_.find(conn);
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
       * should not happen - when connection is not available
       * thread is sleeped until another released one and notify it
       */
      throw Exception("No free connection available!");
    }
    
  }


  /**
   * release connection back to pool; there can't be any transaction
   * active on the connection!
   *
   * @param _conn  connection acquired from this pool
   */
  virtual void release(connection_driver *_conn) {
    boost::mutex::scoped_lock scoped_lock(pool_lock_);

    /* find connection data */
    typename storage_type::iterator it = pool_.find(_conn);
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


  /**
   * Connection string getter
   *
   * @return  connection string factory was configured with
   */
  virtual inline const std::string& getConnectionString() const {
    return conn_info_;
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
      /* TODO: relax down pool (base on what? some usage statistic?) :) */
#ifdef HAVE_LOGGER
      if (_to != pool_.size()) {
        LOGGER(PACKAGE).warning("can't relax pool size down - not enough free connections");
      }
#endif
      return;
    }

    for (unsigned i = pool_.size(); i < _to; ++i) {
      /* create new connection and its info data */
      connection_driver *conn = new connection_driver(this->conn_info_);
      conn_data_ cd(i, false);
      /* store it in pool and enqueue into free */
      pool_.insert(std::make_pair(conn, cd));
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

    typename storage_type::const_iterator it = pool_.begin();
    for (; it != pool_.end(); ++it) {
      if (it->second.used == true)
        used += 1;
      else
        free += 1;
    }

    return str(boost::format("connection pool summary: "
                             "total=%1% (used=%2% free=%3% max=%4%)")
                             % total % used % free % max_conn_);
  }

#ifdef HAVE_LOGGER
  inline void logStatus_() const {
    LOGGER(PACKAGE).info(status_());
  }
#endif


private:
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

  typedef std::map<connection_driver*, conn_data_> storage_type;
  typedef std::queue<connection_driver*>           queue_type;

  std::string  conn_info_;               /**< connection string (host, database name, user, password ...) */
  unsigned     init_conn_;               /**< connections established at init */
  unsigned     max_conn_;                /**< maximum number of connection */

  storage_type             pool_;        /**< connection pool (conn and info data) */
  queue_type               free_;        /**< free conns for quick acces */
  mutable boost::mutex     pool_lock_;   /**< thread safety */
  mutable boost::condition has_free_;    /**< wait condition when no connection available */

};


}
}


#endif /*CONNECTION_FACTORY_H_*/

