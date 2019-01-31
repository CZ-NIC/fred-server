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
 *  Implementation of connection object for Mock database.
 */


#ifndef MOCK_CONNECTION_HH_519CF51C725E43728305C0470BFCDBDC
#define MOCK_CONNECTION_HH_519CF51C725E43728305C0470BFCDBDC

//mock #include <libpq-fe.h>
#include "src/util/db/mock/mock_result.hh"
#include "src/util/db/statement.hh"
#include "util/db/db_exceptions.hh"

namespace Database {


class MockTransaction;

typedef void MOCKconn; //PGconn replacement


/**
 * \class MockConnection
 * \brief Mock connection driver for Connection_ template
 */
class MockConnection {
private:
  std::string   conn_info_;
  MOCKconn       *psql_conn_;         /**< wrapped connection structure from libpq library */
  bool          psql_conn_finish_;  /**< whether or not to finish MOCKconn at the destruct (close() method) */

public:
  typedef MockResult       result_type;
  typedef MockTransaction  transaction_type;

  /**
   * Constructors and destructor
   */
  MockConnection() : psql_conn_(0),
                     psql_conn_finish_(true) {
  }


  MockConnection(const std::string& _conn_info) /* throw (ConnectionFailed) */
               : conn_info_(_conn_info), psql_conn_(0), psql_conn_finish_(true) {
    open(_conn_info);
  }


  MockConnection(MOCKconn *_psql_conn) : psql_conn_(_psql_conn), psql_conn_finish_(false) {
  }


  virtual ~MockConnection() {
    close();
    // std::cout << "(CALL) MockConnection destructor" << std::endl;
  }


  /**
   * Implementation of coresponding methods called by Connection_ template
   */

  virtual void open(const std::string& _conn_info) /* throw (ConnectionFailed) */{
    conn_info_ = _conn_info;
    close();
    /*mock
    psql_conn_ = PQconnectdb(_conn_info.c_str());
    if (PQstatus(psql_conn_) != CONNECTION_OK) {
      PQfinish(psql_conn_);
      throw ConnectionFailed(_conn_info);
    }
    */
  }


  virtual void close() {
    if (psql_conn_ && psql_conn_finish_) {
    	/*mock
      PQfinish(psql_conn_);
      */
      psql_conn_ = 0;
    }
  }


  virtual inline result_type exec(Statement& _query) /*throw (ResultFailed) */{
    return exec(_query.toSql(boost::bind(&MockConnection::escape, this, _1)));
  }


  virtual inline result_type exec(const std::string& _query) /*throw (ResultFailed)*/ {
    DummyResult *tmp = 0;//mock PQexec(psql_conn_, _query.c_str());
    /*mock
    ExecStatusType status = PQresultStatus(tmp);
    if (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK) {
      return MockResult(tmp);
    }
    else {
      //mock PQclear(tmp);
      throw ResultFailed(_query + " (" + PQerrorMessage(psql_conn_) + ")");
    }
    */ return MockResult(tmp);
  }

  virtual inline result_type exec_params(const std::string& _query,//one command query
          const std::vector<std::string>& params //pointer to memory with parameters data
          )
  {
      DummyResult *tmp = 0;
      return MockResult(tmp);
  }

  virtual inline result_type exec_params(const std::string& _query,//one command query
          const QueryParams& params //parameters data
      )
  {
      DummyResult *tmp = 0;
      return MockResult(tmp);
  }

  virtual void reset() {
    //mock PQreset(psql_conn_);
  }


  virtual std::string escape(const std::string &_in) const {
/*mock
	std::string ret;
    char *esc = new char [2*_in.size() + 1];
    int err;

    PQescapeStringConn(psql_conn_, esc, _in.c_str(), _in.size(), &err);
    ret = esc;
    delete [] esc;

    if (err) {
#ifdef HAVE_LOGGER
      LOGGER.error(boost::format("error in escape function: %1%") % PQerrorMessage(psql_conn_));
#endif
    }
      return ret;
*/
    return _in;
  }


  bool is_in_transaction() const {
	/*mock
    PGTransactionStatusType ts = PQtransactionStatus(psql_conn_);
    return ts == PQTRANS_INTRANS || ts == PQTRANS_INERROR;
    */
	  return false;
  }

  virtual inline void setQueryTimeout(unsigned) { }

  /* HACK! HACK! HACK! */
  typedef MOCKconn* __conn_type__;
  __conn_type__ __getConn__() const {
      return psql_conn_;
  }
};



/**
 * \class MockTransaction
 * \brief Implementation of local transaction for Mock driver
 */
class MockTransaction {
public:
  typedef MockConnection connection_type;


	MockTransaction() {
  }


	~MockTransaction() {
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

