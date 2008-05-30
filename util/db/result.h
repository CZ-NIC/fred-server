#ifndef RESULT_H_
#define RESULT_H_

#include <string>
#include <vector>
#include "query.h"
#include "connection.h"
#include "result_it.h"
#include "row.h"

namespace DBase {

class PSQLResult;

/**
 * Abstract interface for result class 
 */
class Result {
public:
	/**
	 * C-tor, D-tor
	 */
	Result(const Connection *_conn, const Query& _query) :
		conn(_conn), query(_query) {
	}
	virtual ~Result();
	/**
	 * Returns query corresponding with result object
	 */
	const Query& getQuery() const {
		return query;
	}
	/**
	 * Getting iterator for iterating over result object
	 */
	virtual ResultIterator* getIterator() = 0;
	/**
	 * Getting number of rows and cols
	 */
	virtual unsigned getNumRows() = 0;
	virtual unsigned getNumCols() = 0;

protected:
	/**
	 * Pointer for connection to which result object belongs to
	 */
	const Connection *conn;
	/**
	 * Result query 
	 */
	Query query;
};

template<class _Tp> class IteratorProxy;

class Result__ {
public:
	Result__(const Connection *_conn, const Query& _query);
	virtual ~Result__();

	class Iterator__;
	friend class Iterator__;
	class Iterator__ :
		public std::iterator<std::forward_iterator_tag, Row> {
	public:
		Iterator__() {
		}
		virtual ~Iterator__() {
		}
		virtual value_type getValue() = 0;
		virtual void next() = 0;
		virtual bool isDone() = 0;
	};

	typedef IteratorProxy<Row> Iterator;

	const Query& getQuery() const;

	virtual Iterator begin() = 0;
	virtual unsigned getNumRows() = 0;
	virtual unsigned getNumCols() = 0;

protected:
	const Connection *conn;
	Query query;

};

template<class _Tp> class IteratorProxy {
public:
	IteratorProxy(Result__::Iterator__ *_it) :
		value(_it) {
	}
	virtual ~IteratorProxy() {
		delete value;
	}
	virtual _Tp getValue() {
		return value->getValue();
	}
	virtual void next() {
		return value->next();
	}
	virtual bool isDone() {
		return value->isDone();
	}
protected:
	Result__::Iterator__ *value;
};

}

#endif /*RESULT_H_*/
