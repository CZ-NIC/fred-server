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
#include <vector>
#include "db_exceptions.h"
#include "result.h"
#include "statement.h"

#include "config.h"

#ifdef HAVE_LOGGER
#include "log/logger.h"
#endif

namespace Database {


/**
 * Forward declaration
 */
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
                                                                    opened_(false),
                                                                    trans_(0) {
     /* lazy connection open */
     if (!_lazy_connect) {
      open(conn_info_);
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
    opened_ = true;
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
  virtual inline Result_<result_type> exec(Statement& _query) throw (ResultFailed) {
    if (!opened_) {
      open(conn_info_);
    }
    return exec(_query.toSql(boost::bind(&Connection_<connection_type>::escape, this, _1)));
  }


  /**
   * @param _query string representation of query
   * @return       result
   */
  virtual inline Result_<result_type> exec(const std::string& _query) throw (ResultFailed) {
    try {
      if (!opened_) {
        open(conn_info_);
      }
#ifdef HAVE_LOGGER
      LOGGER(PACKAGE).debug(boost::format("exec query [%1%]") % _query);
#endif
      return Result_<result_type>(conn_.exec(_query));
    }
    catch (ResultFailed &rf) {
      throw;
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


  virtual bool inTransaction() const {
    return conn_.inTransaction();
  }


  virtual inline std::string escape(const std::string &_in) const {
    return conn_.escape(_in);
  }

  
  template<class _Type>
  friend class Transaction_;


protected:
  /**
   * Transaction information add/remove operation
   * (should be protected and Transaction_ template make friend?)
   */
  inline virtual void setTransaction(Transaction_<transaction_type> *_trans) {
    trans_ = _trans;
#ifdef HAVE_LOGGER
    LOGGER(PACKAGE).debug(boost::format("(%1%) transaction assigned to connection") % trans_);
#endif
  }


  inline virtual void unsetTransaction() {
#ifdef HAVE_LOGGER
    LOGGER(PACKAGE).debug(boost::format("(%1%) transaction released from connection") % trans_);
#endif
    trans_ = 0;
  }


  inline Transaction_<transaction_type>* getTransaction() const {
    return trans_;
  }


private:
  connection_type  conn_;      /**< connection driver */
  std::string      conn_info_; /**< connection string used to open connection */
  bool             opened_;    /**< whether is connection opened or not (for lazy connect) */
  Transaction_<transaction_type> *trans_; /**< active transaction pointer */
};



/**
 * \class Transaction_ 
 * \brief Base template class representing local transaction
 *
 * This implementation uses SAVEPOINTS for dealing with nested transactions 
 * - in Connection_ we store pointer to base (the most top)
 * transaction created
 */
template<class _Type>
class Transaction_ {
public:
  typedef _Type                                       transaction_type;
  typedef typename transaction_type::connection_type  connection_type;
  typedef typename connection_type::result_type       result_type;
  typedef std::vector<std::string>                    savepoint_list;

  
  Transaction_(Connection_<connection_type> &_conn) : conn_(_conn),
                                                      ptransaction_(0),
                                                      success_(false) {
    if (!conn_.inTransaction()) {
#ifdef HAVE_LOGGER
      LOGGER(PACKAGE).debug(boost::format("(%1%) start transaction request -- begin") % this);
#endif
      exec(transaction_.start());
      conn_.setTransaction(this);
    }
    else {
#ifdef HAVE_LOGGER
      LOGGER(PACKAGE).debug(boost::format("(%1%) start transaction request -- (%2%) already active") % this % conn_.getTransaction());
#endif
      setParentTransaction(conn_.getTransaction());
      conn_.setTransaction(this);
      savepoint();
    }
  }


  virtual ~Transaction_() {
    if (!success_) {
      if (!ptransaction_) {
#ifdef HAVE_LOGGER
        LOGGER(PACKAGE).debug(boost::format("(%1%) rollback transaction request -- rollback") % this);
#endif
        exec(transaction_.rollback());
        conn_.unsetTransaction();
      }
      else {
#ifdef HAVE_LOGGER
        LOGGER(PACKAGE).debug(boost::format("(%1%) rollback transaction request -- to savepoint") % this);
#endif
        conn_.setTransaction(ptransaction_);
        exec(transaction_.rollback() + " TO SAVEPOINT " + savepoints_.front());
      }
    }
  }


  virtual void commit() {
    if (ptransaction_) {
#ifdef HAVE_LOGGER
      LOGGER(PACKAGE).debug(boost::format("(%1%) commit transaction request -- release savepoint") % this);
#endif
      conn_.exec("RELEASE SAVEPOINT " + savepoints_.front());
      conn_.setTransaction(ptransaction_);
    }
    else {
      if (conn_.getTransaction() == this) {
#ifdef HAVE_LOGGER
        LOGGER(PACKAGE).debug(boost::format("(%1%) commit transaction request -- commit ok") % this);
#endif
        exec(transaction_.commit());
        conn_.unsetTransaction();
      }
      else {
#ifdef HAVE_LOGGER      
        LOGGER(PACKAGE).error(boost::format("(%1%) commit transaction request -- child active!") % this);
#endif
      }
    }
    success_ = true;
  }

  
  inline Result_<result_type> exec(const std::string &_query) {
    return conn_.exec(_query);
  }


  inline Result_<result_type> exec(Statement &_stmt) {
    return conn_.exec(_stmt); 
  }


  virtual void savepoint(std::string _name = std::string()) {
    if (_name.empty()) {
      _name = generateSavepointName();
    }
    savepoints_.push_back(_name);
    conn_.exec("SAVEPOINT " + _name);
  }


protected:
  inline const savepoint_list::size_type getNextSavepointNum() const {
    return savepoints_.size();
  }


  virtual std::string generateSavepointName() const {
    savepoint_list::size_type num = savepoints_.size();
    if (ptransaction_) {
      num = std::max(num, ptransaction_->getNextSavepointNum());
    }

    return str(boost::format("sp%1%") % num);
  }


  void setParentTransaction(Transaction_<transaction_type> *_trans) {
    ptransaction_ = _trans;
#ifdef HAVE_LOGGER
    LOGGER(PACKAGE).debug(boost::format("(%1%) parent transaction assigned (%2%)") % this % ptransaction_);
#endif
  }


  void unsetParentTransaction() {
#ifdef HAVE_LOGGER
    LOGGER(PACKAGE).debug(boost::format("(%1%) parent transaction released (%2%)") % this % ptransaction_);
#endif
    ptransaction_ = 0;
  }


  Transaction_<transaction_type>* getParentTransaction() const {
    return ptransaction_;
  }


private:
  Connection_<connection_type>   &conn_;
  Transaction_<transaction_type> *ptransaction_;
  transaction_type                transaction_;
  bool                            success_;
  savepoint_list                  savepoints_;

};


}

#endif /*CONNECTION_HPP_*/

