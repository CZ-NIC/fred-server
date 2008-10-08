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
 *  @file connection.h
 *  Interface definition of connection object.
 */


#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include <boost/noncopyable.hpp>
#include <string>
#include "db_exceptions.h"
#include "result.h"
#include "query.h"

#include "config.h"

#ifdef HAVE_LOGGER
#include "log/logger.h"
#endif

namespace Database {

template<class _Type>
class Transaction_;
/**
 * \class Connection_
 * \brief Base template class for represent database connection
 *
 * template for connection object which uses _ConnType class 
 * as a concrete driver.
 *
 * Connection object (which should be used) is defined in \file database.h
 * file.
 */
template<class _ConnType>
class Connection_ : private boost::noncopyable {
public:
  typedef _ConnType                                  connection_type;
  typedef typename connection_type::transaction_type transaction_type;
  typedef typename connection_type::result_type      result_type;

  friend class Transaction_<transaction_type>;

  class ResultFailed : public Exception {
  public:
    ResultFailed(const std::string& _query) : Exception("Result failed: " + _query) { }
  };

  /**
   * Constructors and destructor
   */
  Connection_() : opened_(false) {
  }

  /**
   * @param _conn_info is connection string used to open database 
   */
  Connection_(const std::string& _conn_info,
              bool _lazy_connect = true) throw (ConnectionFailed) : conn_info_(_conn_info), 
                                                                    opened_(false) {
     /* lazy connection open */
     if (!_lazy_connect) {
      open(conn_info_);
      opened_ = true;
     }
  }


  virtual ~Connection_() {
#ifdef HAVE_LOGGER
    TRACE("<CALL> Database::~Connection_()");
#endif
  }

  /**
   * @param _conn_info is connetion string used to open database
   */
  virtual void open(const std::string& _conn_info) throw (ConnectionFailed) {
    conn_info_ = _conn_info;
    conn_.close();
    conn_.open(conn_info_);
#ifdef HAVE_LOGGER
    LOGGER(PACKAGE).info(boost::format("connection established; (%1%)") % conn_info_);
#endif
  }


  virtual void close() {
    conn_.close();
  }

  /**
   * Query executors
   */

  /**
   * This call is converted to stringize method call
   *
   * @param _query object representing query
   * @return       result
   */
  virtual Result_<result_type> exec(Query& _query) throw (ResultFailed) {
    try {
      /* check if query is fully constructed */
      if (!_query.initialized()) {
        _query.make();
      }
      return exec(_query.str());
    }
    catch (...) {
      throw ResultFailed(_query.str());
    }
  }


  /**
   * @param _query string representation of query
   * @return       result
   */
  virtual Result_<result_type> exec(const std::string& _query) throw (ResultFailed) {
    try {
      if (!opened_) {
        open(conn_info_);
        opened_ = true;
      }
#ifdef HAVE_LOGGER
      LOGGER(PACKAGE).debug(boost::format("exec query [%1%]") % _query);
#endif
      return Result_<result_type>(conn_.exec(_query));
    }
    catch (...) {
      throw ResultFailed(_query);
    }
  }


  /**
   * Reset connection to state after connect
   */
  virtual void reset() {
    conn_.reset(conn_info_);
  }


private:
  connection_type conn_;      /**< connection driver */
  std::string     conn_info_; /**< connection string used to open connection */
  bool            opened_;    /**< whether is connection opened or not (for lazy connect) */
};



/**
 * \class Transaction_ 
 * \brief Base template class representing local transaction
 */
template<class _Type>
class Transaction_ {
public:
  typedef _Type                                       transaction_type;
  typedef typename transaction_type::connection_type  connection_type;
  typedef typename connection_type::result_type       result_type;
	
  Transaction_(Connection_<connection_type> &_conn) : conn_(_conn),
                                                      success_(false) {
    Query _q(transaction_.start());
    exec(_q);
  }


	virtual ~Transaction_() {
    if (!success_) {
      Query _q = transaction_.rollback();
      exec(_q);
    }
  }


	void commit() {
    Query _q = transaction_.commit();
    exec(_q);
    success_ = true;
  }

  
  Result_<result_type> exec(Query &_query) {
    return conn_.exec(_query);
  }


private:
  Connection_<connection_type> &conn_;
  transaction_type             transaction_;
  bool                         success_;
};


}

#endif /*CONNECTION_HPP_*/

