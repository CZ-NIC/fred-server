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
 *  @file psql_connection.h
 *  Implementation of connection object for PSQL database.
 */


#ifndef PSQL_CONNECTION_H_
#define PSQL_CONNECTION_H_

#include <libpq-fe.h>
#include <algorithm>
#include "psql_result.h"
#include "../statement.h"
#include "../db_exceptions.h"

namespace Database {

#ifdef HAVE_LOGGER
static void logger_notice_processor(void *arg, const char *message)
{
    // replace new line symbols
    std::string msg(message);
    std::replace(msg.begin(), msg.end(), '\n', ' ');
    LOGGER(PACKAGE).debug(msg);
}
#endif

class PSQLTransaction;

/**
 * \class PSQLConnection
 * \brief PSQL connection driver for Connection_ template
 */
class PSQLConnection {
private:
  std::string   conn_info_;
  PGconn       *psql_conn_;         /**< wrapped connection structure from libpq library */
  bool          psql_conn_finish_;  /**< whether or not to finish PGconn at the destruct (close() method) */

public:
  typedef PSQLResult       result_type;
  typedef PSQLTransaction  transaction_type;

  /**
   * Constructors and destructor
   */
  PSQLConnection() : psql_conn_(0),
                     psql_conn_finish_(true) {
  }


  PSQLConnection(const std::string& _conn_info) /* throw (ConnectionFailed) */
               : conn_info_(_conn_info), psql_conn_(0), psql_conn_finish_(true) {
    open(_conn_info);
  }


  PSQLConnection(PGconn *_psql_conn) : psql_conn_(_psql_conn), psql_conn_finish_(false) {
  }


  virtual ~PSQLConnection() {
    close();
    // std::cout << "(CALL) PSQLConnection destructor" << std::endl;
  }


  /**
   * Implementation of coresponding methods called by Connection_ template
   */

  virtual void open(const std::string& _conn_info) /* throw (ConnectionFailed) */{
    conn_info_ = _conn_info;
    close();
    psql_conn_ = PQconnectdb(_conn_info.c_str());
    if (PQstatus(psql_conn_) != CONNECTION_OK) {
    	std::string err_msg =  PQerrorMessage(psql_conn_);
      PQfinish(psql_conn_);
      throw ConnectionFailed(_conn_info + " errmsg: " + err_msg);
    }
#ifdef HAVE_LOGGER
    // set notice processor
    PQsetNoticeProcessor(psql_conn_, logger_notice_processor, NULL);
#endif
  }


  virtual void close() {
    if (psql_conn_ && psql_conn_finish_) {
      PQfinish(psql_conn_);
      psql_conn_ = 0;
    }
  }


  virtual inline result_type exec(Statement& _query) /*throw (ResultFailed) */{
    return exec(_query.toSql(boost::bind(&PSQLConnection::escape, this, _1)));
  }


  virtual inline result_type exec(const std::string& _query) /*throw (ResultFailed)*/ {
    PGresult *tmp = PQexec(psql_conn_, _query.c_str());

    ExecStatusType status = PQresultStatus(tmp);
    if (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK) {
      return PSQLResult(tmp);
    }
    else {
      PQclear(tmp);
      throw ResultFailed(_query + " (" + PQerrorMessage(psql_conn_) + ")");
    }
  }

  virtual inline result_type exec_params(const std::string& _query,//one command query
          const std::vector< const char * > paramValues, //pointer to memory with parameters data
          const std::vector<int> paramLengths, //sizes of memory with parameters data
          const std::vector<int> paramFormats,//1-binary like int or double, 0- text like const char *
          int resultFormat=0) //1-binary result so far not supported , 0- text result
  {
      if(paramValues.size() != paramLengths.size())
          throw std::runtime_error(
                  "runtime error: paramValues and paramLengths have different size");
      if(paramValues.size() != paramFormats.size())
          throw std::runtime_error(
                  "runtime error: paramValues and paramFormats have different size");
  //example at:
  //http://timmurphy.org/2009/11/19/pqexecparams-example-postgresql-query-execution-with-parameters
    PGresult *tmp = PQexecParams(psql_conn_, _query.c_str()//query buffer
        , paramValues.size()//number of parameters
        , 0 //not using Oids, use type in query like: WHERE id = $1::int4 and name = $2::varchar
        , &paramValues[0]//values to substitute $1 ... $n
        , &paramLengths[0]//the lengths, in bytes, of each of the parameter values
        , &paramFormats[0]//whether the values are binary or not
        , resultFormat);////we want the result in text format

    ExecStatusType status = PQresultStatus(tmp);
    if (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK)
    {
      return PSQLResult(tmp);
    }
    else
    {
      PQclear(tmp);
      throw ResultFailed(_query + " (" + PQerrorMessage(psql_conn_) + ")");
    }
  }//exec_params


  virtual void reset() {
    PQreset(psql_conn_);
  }


  virtual std::string escape(const std::string &_in) const {
    std::string ret;
    char *esc = new char [2*_in.size() + 1];
    int err;

    PQescapeStringConn(psql_conn_, esc, _in.c_str(), _in.size(), &err);
    ret = esc;
    delete [] esc;

    if (err) {
      /* error */
#ifdef HAVE_LOGGER
      LOGGER(PACKAGE).error(boost::format("error in escape function: %1%") % PQerrorMessage(psql_conn_));
#endif
    }

    return ret;
  }


  bool inTransaction() const {
    PGTransactionStatusType ts = PQtransactionStatus(psql_conn_);
    return ts == PQTRANS_INTRANS || ts == PQTRANS_INERROR;
  }



  /* HACK! HACK! HACK! */
  typedef PGconn* __conn_type__;
  __conn_type__ __getConn__() const {
      return psql_conn_;
  }
};



/**
 * \class PSQLTransaction
 * \brief Implementation of local transaction for PSQL driver
 */
class PSQLTransaction {
public:
  typedef PSQLConnection connection_type;


	PSQLTransaction() {
  }


	~PSQLTransaction() {
  }


  inline std::string start() {
    return "START TRANSACTION  ISOLATION LEVEL READ COMMITTED";
  }


  inline std::string rollback() {
    return "ROLLBACK TRANSACTION";
  }


	inline std::string commit() {
    return "COMMIT TRANSACTION";
  }
};



}

#endif /*PSQL_CONNECTION_H*/

