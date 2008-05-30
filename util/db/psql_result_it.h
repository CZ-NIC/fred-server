#ifndef PSQL_RESULT_IT_H_
#define PSQL_RESULT_IT_H_

#include <libpq-fe.h>
#include <string>
#include "result.h"
#include "psql_result.h"

namespace DBase {

class PSQLResultIterator : public ResultIterator {
protected:
	const PSQLResult* result_ptr;
	unsigned row;
	unsigned col;

public:
	PSQLResultIterator(const PSQLResult* _result);
	~PSQLResultIterator() {
	}
	void first() {
		row = 0;
		col = 0;
	}
	void next();
	bool isDone();
	const Value getNextValue();
	bool isNextValue();
};

}

#endif /*PSQL_RESULT_IT_H_*/
