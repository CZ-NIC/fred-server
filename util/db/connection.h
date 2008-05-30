#ifndef DB_CONNECTION_H_
#define DB_CONNECTION_H_

#include <string>
#include "dbexceptions.h"
#include "query.h"

namespace DBase {

class Result;
class Result__;
class Transaction;

/**
 * Abstract interface for connection class
 */
class Connection {
public:
	/**
	 * C-tor, D-tor
	 */
	Connection(std::string _conn_info) throw (ConnectionFailed) :
		conn_info(_conn_info), transaction_active(false) {
	}
	virtual ~Connection();
	/**
	 * Execute query given as parameter
	 */
	virtual Result* exec(Query& _query) = 0;
	virtual void exec(InsertQuery& _query) = 0;
	virtual Result__* exec__(Query& _query) = 0;
	/**
	 * Local transaction support
	 */
	virtual Transaction getTransaction() = 0;
	friend class Transaction;
	/**
	 * Info about database backend
	 * (not so important if it isnt't available in library can have
	 * some dummy implementation)
	 */
	virtual int serverVersion() const = 0;
	virtual int protocolVersion() const = 0;

protected:
	/**
	 * Interface for internal transaction implementation
	 * called by Transaction object 
	 */
	virtual void beginTransaction() = 0;
	virtual void endTransaction() = 0;
	virtual void rollbackTransaction() = 0;
	virtual void commitTransaction() = 0;

	/**
	 * Escaping string support (not in use now)
	 */
	virtual std::string escapeString(const std::string& _str) const = 0;

	/**
	 * Stored connection string
	 */
	std::string conn_info;
	/**
	 * Transaction status (active or not)
	 */
	bool transaction_active;
};

}

#endif /*DB_CONNECTION_H_*/
