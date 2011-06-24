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
 *  Interface definition of automanaged connection object
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <boost/noncopyable.hpp>
#include <string>
#include <vector>
#include "db_exceptions.h"
#include "result.h"
#include "statement.h"
#include "connection_factory.h"
#include "query_param.h"

#include "config.h"

#ifdef HAVE_LOGGER
#include "log/logger.h"
#endif


namespace Database {

/**
 * \class  ConnectionBase_
 */
template<class connection_driver, class manager_type>
class ConnectionBase_ {
public:
  typedef connection_driver                        driver_type;
  typedef typename manager_type::transaction_type  transaction_type;
  typedef typename manager_type::result_type       result_type;

  /**
   * Constructor and destructor
   */
  ConnectionBase_(connection_driver *_conn) 
            : conn_(_conn) {
  }


  virtual ~ConnectionBase_() {
  }

  static const std::string getTimeoutString() {
      return connection_driver::getTimeoutString();
  }


  /**
   * Query executors
   */

  /**
   * This call is converted to stringize method call
   *
   * @param _query object representing query statement
   * @return       result
   */
  virtual inline result_type exec(Statement& _stmt) /*throw (ResultFailed)*/ {
    return this->exec(_stmt.toSql(boost::bind(&ConnectionBase_<connection_driver, manager_type>::escape, this, _1)));
  }

  /**      
   * @param _query boost::format representing query statement
   * @return       result
   */
  virtual inline result_type exec(const boost::format &_fmt) /*throw (ResultFailed)*/ {
      try {
        return exec(_fmt.str());
      } catch (ResultFailed &rf) {
          throw;
      } catch (...) {
          throw ResultFailed(">>Conversion error<<");
      }
  }

  /**
   * @param _query string representation of query statement
   * @return       result
   */
  virtual inline result_type exec(const std::string &_stmt) /*throw (ResultFailed)*/ {
    try {
#ifdef HAVE_LOGGER
      LOGGER(PACKAGE).debug(boost::format("exec query [%1%]") % _stmt);
#endif
      return result_type(conn_->exec(_stmt));
    }
    catch (ResultFailed &rf) {
      throw;
    }
    catch (...) {
      throw ResultFailed(_stmt);
    }
  }
  /**
   * @param _stmt string representation of query statement with params
   * @param params vector of param data strings
   * @return result
   */

  virtual inline result_type exec_params(const std::string& _stmt,//one command query
          const std::vector<std::string>& params //parameters data
		  )
  {
      try {
  #ifdef HAVE_LOGGER
        LOGGER(PACKAGE).debug(boost::format("exec query [%1%]") % _stmt);
  #endif
        return result_type(conn_->exec_params(_stmt//one command query
                          , params //parameters data
                          ));
      }
      catch (ResultFailed &rf) {
        throw;
      }
      catch (...) {
        throw ResultFailed(_stmt);
      }
    }

  /**
   * @param _stmt string representation of query statement with params
   * @param params vector of query param data strings or binary
   * @return result
   */

  virtual inline result_type exec_params(const std::string& _stmt,//one command query
          const QueryParams& params //parameters data
          )
  {
      try {
  #ifdef HAVE_LOGGER

          if(LOGGER(PACKAGE).getLevel() >= Logging::Log::LL_DEBUG)
          {
              std::string value;
              std::string params_dump;
              std::size_t params_counter =0;
              for (QueryParams::const_iterator i = params.begin()
                      ; i != params.end() ; ++i)
              {
                  ++params_counter;
                  value = i->is_null() ? "[null]" : "'" + i->print_buffer() + "'";
                  params_dump += std::string(" $")
                      + boost::lexical_cast<std::string>(params_counter) + ": "
                      + value;
              }//for params

              LOGGER(PACKAGE).debug(
                      boost::format("exec query [%1%] params %2%")
                  % _stmt % params_dump);
          }//if debug

  #endif
        return result_type(conn_->exec_params(_stmt//one command query
                          , params //parameters data
                          ));
      }
      catch (ResultFailed &rf) {
        throw;
      }
      catch (...) {
        throw ResultFailed(_stmt);
      }
    }

  
  /**
   * Reset connection to state after connect
   */
  virtual inline void reset() {
    conn_->reset();
  }


  /**
   * String escape method by specific connection_driver
   */
  virtual inline std::string escape(const std::string &_in) {
    return conn_->escape(_in);
  }


  /**
   * @return  true if there is active transaction on connection
   *          false otherwise
   */
  virtual inline bool inTransaction() const {
    return conn_->inTransaction();
  }

  virtual inline void setConstraintExclusion(bool on = true) {
    conn_->setConstraintExclusion(on);
  }

  virtual inline void setQueryTimeout(unsigned t) {
    conn_->setQueryTimeout(t);
#ifdef HAVE_LOGGER
      LOGGER(PACKAGE).debug(boost::format("sql statement timout set to %1%ms") % t);
#endif
  }

  /* HACK! HACK! HACK! (use with construct with old DB connection) */
  typename driver_type::__conn_type__ __getConn__() const {
    return conn_->__getConn__();
  }


protected:
  connection_driver       *conn_;    /**< connection_driver instance */
};


/**
 * \class  Connection_
 * \brief  Standard connection proxy class
 */
template<class connection_driver, class manager_type>
class Connection_ : public ConnectionBase_<connection_driver, manager_type> {
public:
  typedef ConnectionBase_<connection_driver, manager_type>   super;
  typedef typename super::driver_type                        driver_type;
  typedef typename super::result_type                        result_type;
  typedef typename super::transaction_type                   transaction_type;


  /**
   * Conectructors
   */
  Connection_() : super(0), trans_(0) {
  }


  Connection_(const std::string &_conn_info,
              bool _lazy_connect = true) //throw (ConnectionFailed)
            : super(0),
              trans_(0),
              conn_info_(_conn_info) {

    /* lazy connection open */
    if (!_lazy_connect) {
      open(conn_info_);
    }
  }


  Connection_(connection_driver *_conn) : super(_conn), trans_(0) {
  }


  /**
   * Destructor
   *
   * close connection on destruct
   */
  virtual ~Connection_() {
    close();
  }


  /**
   * Open connection with specific connection string
   */
  virtual void open(const std::string &_conn_info) /* throw (ConnectionFailed) */{
    close();
    this->conn_info_ = _conn_info;
    /* TODO: this should be done by manager_type! */
    this->conn_ = new connection_driver(conn_info_);
#ifdef HAVE_LOGGER
    LOGGER(PACKAGE).info(boost::format("connection established; (%1%)") % conn_info_);
#endif
  }


  /**
   * Close connection
   */
  virtual inline void close() {
    if (this->conn_) {
      delete this->conn_;
      this->conn_ = 0;
#ifdef HAVE_LOGGER
      try{
          LOGGER(PACKAGE).info(boost::format("connection closed; (%1%)") % conn_info_);
      }catch(...){}
#endif
    }
  }


  /**
   * Exec query statement
   * 
   * Need to check if connection was opened - support for lazy connection opening
   */
  virtual inline result_type exec(Statement &_stmt) /*throw (ResultFailed)*/ {
    if (!this->conn_) {
      open(conn_info_);
    }
    return super::exec(_stmt);
  }


  virtual inline result_type exec(const std::string &_stmt) /*throw (ResultFailed)*/ {
    if (!this->conn_) {
      open(conn_info_);
    }
    return super::exec(_stmt);
  }

  virtual inline std::string escape(const std::string &_in) {
    if (!this->conn_) {
      open(conn_info_);
    }
    return super::escape(_in);
  }


  template<class _transaction_type, class _manager_type>
  friend class Transaction_;


protected:
  /**
   * Trasaction support methods
   */
  virtual inline void setTransaction(transaction_type *_trans) {
    trans_ = _trans;
#ifdef HAVE_LOGGER
    LOGGER(PACKAGE).debug(boost::format("(%1%) transaction assigned to (%2%) connection") % trans_ % this->conn_);
#endif
  }


  virtual inline void unsetTransaction() {
#ifdef HAVE_LOGGER
    LOGGER(PACKAGE).debug(boost::format("(%1%) transaction released from connection") % trans_);
#endif
    trans_ = 0;
  }


  virtual inline transaction_type* getTransaction() const {
    return trans_;
  }

  
  transaction_type *trans_;  /**< pointer to active transaction */


private:
  std::string conn_info_;    /**< connection string used to open connection */
};



/**
 * \class  TSSConnection_
 * \brief  Specialized connection proxy class for use with TSSManager_
 *         (static call in destructor)
 */
template<class connection_driver, class manager_type>
class TSSConnection_ : public ConnectionBase_<connection_driver, manager_type> {
public:
  typedef ConnectionBase_<connection_driver, manager_type>   super;
  typedef typename super::transaction_type                   transaction_type;

  /**
   * Constructor and destructor
   */
  TSSConnection_(connection_driver *_conn, transaction_type *&_trans) 
               : super(_conn) {
    this->trans_ = &_trans;
  }


  virtual ~TSSConnection_() {
    // Connection should be returned to the pool either explicitly or at thread exit
    // manager_type::release();
  }


  template<class _transaction_type, class _manager_type>
  friend class Transaction_;


protected:
  /**
   * Trasaction support methods
   */
  virtual inline void setTransaction(transaction_type *_trans) {
    *trans_ = _trans;
#ifdef HAVE_LOGGER
    LOGGER(PACKAGE).debug(boost::format("(%1%) transaction assigned to (%2%) connection") % trans_ % this->conn_);
#endif
  }


  virtual inline void unsetTransaction() {
#ifdef HAVE_LOGGER
    LOGGER(PACKAGE).debug(boost::format("(%1%) transaction released from connection") % trans_);
#endif
    *trans_ = 0;
  }


  virtual inline transaction_type* getTransaction() const {
    return *trans_;
  }


  transaction_type **trans_;  /**< pointer to active transaction */
};


}


#endif /*CONNECTION_H_*/

