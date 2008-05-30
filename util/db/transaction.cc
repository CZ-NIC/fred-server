#include "transaction.h"

namespace DBase {

Transaction::Transaction(Connection& _conn) :
	conn(_conn), success(false) {
	conn.beginTransaction();
	conn.transaction_active = true;
}

Transaction::~Transaction() {
	if (!success) {
		conn.rollbackTransaction();
	}
	conn.transaction_active = false;
}

void Transaction::commit() {
	conn.commitTransaction();
	success = true;
}

}
