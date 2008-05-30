#include <vector>
#include "psql_result.h"

namespace DBase {

PSQLResult::PSQLResult(const Connection* _conn, const Query& _query,
		PGresult *_psql_result) :
	Result(_conn, _query), psql_result(_psql_result) {
	max_row = PQntuples(psql_result);
	max_col = PQnfields(psql_result);
}

PSQLResult::~PSQLResult() {
	PQclear(psql_result);
}

ResultIterator* PSQLResult::getIterator() {
	return new PSQLResultIterator(this);
}

PSQLResult__::PSQLResult__(const Connection* _conn, const Query& _query,
		PGresult *_psql_result) :
	Result__(_conn, _query), psql_result(_psql_result) {
	max_row = PQntuples(psql_result);
	max_col = PQnfields(psql_result);
}

PSQLResult__::~PSQLResult__() {
	PQclear(psql_result);
}

Result__::Iterator PSQLResult__::begin() {
	return Iterator(new Iterator__(this));
}

unsigned PSQLResult__::getNumRows() {
	return max_row;
}
unsigned PSQLResult__::getNumCols() {
	return max_col;
}

const Row PSQLResult__::fetchRow(unsigned _num) const {
	std::vector<Value> tmp;
	for (unsigned n = 0; n < max_col; ++n) {
		Value v(PQgetvalue(psql_result, _num, n), PQgetisnull(psql_result, _num, n));
		tmp.push_back(v);
	}
	return Row(this, tmp);
}

}
