#include "psql_result_it.h"

namespace DBase {

PSQLResultIterator::PSQLResultIterator(const PSQLResult* _result) :
	ResultIterator(), result_ptr(_result), row(0), col(0) {
}

bool PSQLResultIterator::isDone() {
	return row == result_ptr->max_row;
}

const Value PSQLResultIterator::getNextValue() {
	if (col < result_ptr->max_col) {
		const std::string tmp = PQgetvalue(result_ptr->psql_result, row, col);
		const int null = PQgetisnull(result_ptr->psql_result, row, col);
		col += 1;
		return Value(tmp, null);
	}
	throw FieldOutOfRange(result_ptr->query.str(), col, result_ptr->max_col);
}

void PSQLResultIterator::next() {
	row += 1;
	col = 0;
}

bool PSQLResultIterator::isNextValue() {
	return col < result_ptr->max_col;
}

}
