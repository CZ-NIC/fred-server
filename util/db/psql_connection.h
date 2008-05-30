#ifndef PSQL_CONNECTION_H_
#define PSQL_CONNECTION_H_

#include <libpq-fe.h>
#include "dbexceptions.h"
#include "connection.h"
#include "psql_result.h"
#include "transaction.h"

namespace DBase {

/**
 * Connection implementation for PSQL database - OO-wrapper for libpq
 * library.
 */
class PSQLConnection : public Connection {
public:
	/**
	 * Implementing Connection class interface see connection.h
	 */
	PSQLConnection(std::string _conn_info) throw (ConnectionFailed);
	PSQLConnection(const PSQLConnection& _conn);
	~PSQLConnection();
	Result* exec(Query& _query) throw (FatalError, ResultQueryFailed);
	void exec(InsertQuery& _query) throw (FatalError, ResultQueryFailed);
	Result__* exec__(Query& _query) throw (FatalError, ResultQueryFailed);
	Transaction getTransaction();
	
	int serverVersion() const;
	int protocolVersion() const;

protected:
	/**
	 * Implementing Connection class interface see connection.h
	 */
	void beginTransaction();
	void endTransaction();
	void rollbackTransaction();
	void commitTransaction();
	/**
	 * String escaping using libpq function
	 */
	std::string escapeString(const std::string& _str) const;

private:
	/**
	 * Object initialization
	 */
	void _init(const std::string& _conn_info) throw (ConnectionFailed);
	/**
	 * Execution method
	 */
	PGresult* _exec(Query& _query) throw (FatalError, ResultQueryFailed);

	/**
	 * libpq PGconn object pointer representing connection
	 */
	PGconn *psql_conn;
};

}
#endif /*PSQL_CONNECTION_H_*/
