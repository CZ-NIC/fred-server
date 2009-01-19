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
 *  @file connection_tss.h
 *  \brief Storing Connection as a thread specific data
 */

#ifndef MANAGER_TSS_H_
#define MANAGER_TSS_H_

#include <string>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>

#include "singleton.h"
#include "connection.h"
#include "result.h"


namespace Database {


/* forward declaration */
template<class _Type, class _ConnFactory>
class TSSTransaction_; 



/**
 * \class TSSManager
 * \brief Database connection manager for storing and retrieving connections
 *        from thread local data
 *
 * @param _ConnFactory  connection factory class
 */
template<class _ConnFactory>
class TSSManager {
public:
  typedef _ConnFactory                                                                          connection_factory_type;
  typedef typename connection_factory_type::connection_type                                     connection_type;
  typedef TSSTransaction_<typename connection_type::transaction_type, connection_factory_type>  transaction_type;
  typedef typename connection_factory_type::result_type                                         result_type;
  typedef typename connection_factory_type::row_type                                            row_type;


  /**
   * Connection factory object initialization
   *
   * @param _conn_factory  factory to be assigned to manager; it has to be already initialized
   */
  static void init(connection_factory_type *_conn_factory) {
    conn_factory_ = _conn_factory;
    init_ = true;
  }

  /**
   * Acquire database connection for actual thread
   *
   * @return  database connection handler pointer
   */
  static connection_type* acquire() {
    PerThreadData_ *tmp = data_.get();
    if (tmp) {
      return getConnection_(tmp);
    }
    else {
      data_.reset(new PerThreadData_());
      return getConnection_(data_.get());
    }
  }


  /**
   * Explicit release database connection for actual thread
   * back to pool
   */
  static void release() {
    PerThreadData_ *tmp = data_.get();
    if (tmp && tmp->conn) {
        conn_factory_->release(tmp->conn);
    }
  }


private:
  TSSManager() {
  }


  ~TSSManager() {
  }


  /**
   * Stucture to store per thread connection
   * TODO: may be redundand - it could be enough to use `connection_type'
   *       as a template parameter to boost::thread_specific_ptr<>
   */
  struct PerThreadData_ {
    PerThreadData_() : conn(0) { }

    ~PerThreadData_() {
      if (conn) {
        conn_factory_->release(conn);
      }
    }

    connection_type *conn;
  };


  /**
   * Helper method for acquiring database connection
   */
  static connection_type* getConnection_(PerThreadData_ *_data) {
    if (!_data->conn) {
      _data->conn = conn_factory_->acquire();
    }
    return _data->conn;
  }
  

  static connection_factory_type                    *conn_factory_; /**< connection factory */
  static boost::thread_specific_ptr<PerThreadData_>  data_;         /**< data per thread structure */
  static bool                                        init_;         /**< data ready initialized internal flag */
};

template<class _ConnFactory> _ConnFactory* TSSManager<_ConnFactory>::conn_factory_(0);
template<class _ConnFactory> boost::thread_specific_ptr<typename TSSManager<_ConnFactory>::PerThreadData_> TSSManager<_ConnFactory>::data_;
template<class _ConnFactory> bool TSSManager<_ConnFactory>::init_ = false;



/**
 * \class TSSTransaction
 * \brief Transaction extension for using TSSManager 
 *        - default parameter assigned (connection retrived by TSSManager)
 *
 * @param _Type         transaction type - driver
 * @param _ConnFactory  connection factory type
 */
template<class _Type, class _ConnFactory>
class TSSTransaction_ : public Transaction_<_Type> {
public:
  typedef TSSManager<_ConnFactory>          manager_type;
  typedef Transaction_<_Type>               super;
  typedef typename super::transaction_type  transaction_type;
  typedef typename super::connection_type   connection_type;
  typedef typename super::result_type       result_type;

  /**
   * Constructor with default parameter
   */
  TSSTransaction_(Connection_<connection_type> &_conn = *(manager_type::acquire()))
                : super(_conn) { }
};

}


#endif /*MANAGER_TSS_H_*/

