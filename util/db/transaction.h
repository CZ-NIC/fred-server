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
 *  @file transaction.h
 *  Interface definition of local automanaged transaction object
 */


#ifndef TRANSACTION_H_
#define TRANSACTION_H_ 

#include <boost/noncopyable.hpp>
#include <string>
#include <vector>
#include "db_exceptions.h"
#include "result.h"
#include "statement.h"
#include "connection.h"

#include "config.h"

#ifdef HAVE_LOGGER
#include "log/logger.h"
#endif

namespace Database {


/**
 * \class Transaction_ 
 * \brief Base template class representing local transaction
 *
 * This implementation uses SAVEPOINTS for dealing with nested transactions 
 * - in Connection_ we store pointer to base (the most top)
 * transaction created
 */
template<class transaction_type, class manager_type>
class Transaction_ {
public:
  typedef typename manager_type::connection_type      connection_type;
  typedef typename manager_type::result_type          result_type;
  typedef std::vector<std::string>                    savepoint_list;

  
  Transaction_(connection_type &_conn) : conn_(_conn),
                                         ptransaction_(0),
                                         exited_(false) {
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
    rollback();
  }


  virtual void rollback() throw() {
    if (!exited_) {
      try {
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
      } catch(Database::Exception &ex) {
#ifdef HAVE_LOGGER
            LOGGER(PACKAGE).debug(boost::format("(%1%) Rollback failed: %2% ") % this % ex.what());
#endif
      } catch(...) {
#ifdef HAVE_LOGGER
            LOGGER(PACKAGE).debug(boost::format("(%1%) rollback failed - unknown excepiton") % this );
#endif
      }
      exited_ = true;
    }
  }


  virtual void commit() {
    if (!exited_) {
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
      exited_ = true;
    }
  }


  virtual void prepare(const std::string &_id) {
    if (ptransaction_) {
        throw std::runtime_error("cannot call prepare transaction on nested transaction");
    }
    if (_id.empty()) {
        throw std::runtime_error("cannot call prepare transaction without id");
    }

    if (!exited_) {
        exec(transaction_.prepare(_id));
        conn_.unsetTransaction();
        exited_ = true;
    }
  }


  inline result_type exec(const std::string &_query) {
    return conn_.exec(_query);
  }


  inline result_type exec(Statement &_stmt) {
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
  inline savepoint_list::size_type getNextSavepointNum() const {
    return savepoints_.size();
  }


  virtual std::string generateSavepointName() const {
    savepoint_list::size_type num = savepoints_.size();
    if (ptransaction_) {
      num = std::max(num, ptransaction_->getNextSavepointNum());
    }

    return str(boost::format("sp%1%") % num);
  }


  void setParentTransaction(Transaction_<transaction_type, manager_type> *_trans) {
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


  Transaction_<transaction_type, manager_type>* getParentTransaction() const {
    return ptransaction_;
  }


private:
  connection_type                              &conn_;
  Transaction_<transaction_type, manager_type> *ptransaction_;
  transaction_type                              transaction_;
  bool                                          exited_;
  savepoint_list                                savepoints_;
};


}

#endif /*TRANSACTION_H_*/

