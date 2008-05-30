#include <iostream>
#include "psql_connection.h"
#include "log/logger.h"
#include "time_clock.h"

namespace DBase {

/**
 * C-tor, D-tor and initialization
 */
PSQLConnection::PSQLConnection(std::string _conn_info) throw (ConnectionFailed) :
	Connection(_conn_info) {
	_init(conn_info);
}

PSQLConnection::PSQLConnection(const PSQLConnection& _conn) :
	Connection(_conn.conn_info) {
	_init(conn_info);
}

PSQLConnection::~PSQLConnection() {
	PQfinish(psql_conn);
}

void PSQLConnection::_init(const std::string& _conn_info)
		throw (ConnectionFailed) {
	psql_conn = PQconnectdb(conn_info.c_str());
	if (PQstatus(psql_conn) != CONNECTION_OK)
		throw ConnectionFailed(conn_info);
	LOGGER("db").info(boost::format("(%1%) connection established; server version: %2% protocol version: %3%")
			% _conn_info % serverVersion() % protocolVersion());
}

/**
 * Transaction SQL commands definition
 */
Transaction PSQLConnection::getTransaction() {
	return Transaction(*this);
}

void PSQLConnection::beginTransaction() {
	Query q = Query("START TRANSACTION  ISOLATION LEVEL READ COMMITTED");
	Result *tmp = exec(q);
	delete tmp;
}

void PSQLConnection::endTransaction() {
	Query q = Query("END TRANSACTION");
	Result *tmp = exec(q);
	delete tmp;
}

void PSQLConnection::rollbackTransaction() {
	Query q = Query("ROLLBACK TRANSACTION");
	Result *tmp = exec(q);
	delete tmp;
}

void PSQLConnection::commitTransaction() {
	Query q = Query("COMMIT TRANSACTION");
	Result *tmp = exec(q);
	delete tmp;
}

/**
 * String escaping using libpq function
 */
std::string PSQLConnection::escapeString(const std::string& _str) const {
	char* tmp = new char[3 * _str.length()];
	int error = 0;
	PQescapeStringConn(psql_conn, tmp, _str.c_str(), _str.length(), &error);
	if (error == 1) {
		throw EscapeStringFailed(_str);
	}
	return std::string(tmp);
}

/**
 * Query execution method 
 */
PGresult* PSQLConnection::_exec(Query& _query) throw (FatalError,
		ResultQueryFailed) {
	if (!_query.initialized()) {
		_query.make();
	}
	LOGGER("db").debug(boost::format("exec query [%1%]") % _query.str());
	
	TimeClock timer;
	timer.start();
	PGresult *r = PQexec(psql_conn, (_query.str()).c_str());
	timer.stop();
	
	ExecStatusType status = PQresultStatus(r);
	if (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK) {
		LOGGER("db").debug(boost::format("result ok in %1% ms. (rows, cols) = (%2%, %3%)")
				% timer.time() % PQntuples(r) % PQnfields(r));
		return r;
	} else if (status == PGRES_FATAL_ERROR) {
		PQclear(r);
		if (transaction_active)
			throw ResultQueryTransactionFailed(_query.str(), std::string(PQresultErrorMessage(r)));
		else
			throw ResultQueryFailed(_query.str(), std::string(PQresultErrorMessage(r)));
	} else {
		PQclear(r);
		throw UnknownError();
	}
}

/**
 * Wrappers for _exec method
 */
Result* PSQLConnection::exec(Query& _query) throw (FatalError,
		ResultQueryFailed) {
	return new PSQLResult(this, _query, _exec(_query));
}

void PSQLConnection::exec(InsertQuery& _query) throw (FatalError,
    ResultQueryFailed) {
  std::auto_ptr<Result> result(new PSQLResult(this, _query, _exec(_query)));
}

/**
 * Testing new Result class (Changed Iterator concept)
 */
Result__* PSQLConnection::exec__(Query& _query) throw (FatalError,
		ResultQueryFailed) {
	return new PSQLResult__(this, _query, _exec(_query));
}

int PSQLConnection::serverVersion() const {
	return PQserverVersion(psql_conn);
}

int PSQLConnection::protocolVersion() const {
	return PQprotocolVersion(psql_conn);
}

}
