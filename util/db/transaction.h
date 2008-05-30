#ifndef TRANSACTION_H_
#define TRANSACTION_H_

#include "connection.h"

namespace DBase {

/**
 * Class representing local transaction
 */
class Transaction {
private:
	Connection& conn;
	bool success;

public:
	Transaction(Connection& _conn);
	~Transaction();
	void commit();

};

}

#endif /*TRANSACTION_H_*/
