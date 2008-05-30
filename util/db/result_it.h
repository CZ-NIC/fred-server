#ifndef RESULT_IT_H_
#define RESULT_IT_H_

#include "db_value.h"

namespace DBase {

class ResultIterator {
protected:

public:
	ResultIterator() {
	}
	virtual ~ResultIterator() {
	}
	virtual void first() = 0;
	virtual void next() = 0;
	virtual bool isDone() = 0;
	virtual bool isNextValue() = 0;
	virtual const Value getNextValue() = 0;
};

}
#endif /*RESULT_IT_H_*/
