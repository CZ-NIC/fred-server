#ifndef PSQL_RESULTH_H_
#define PSQL_RESULTH_H_

#include <libpq-fe.h>
#include "connection.h"
#include "result.h"
#include "psql_result_it.h"
#include "row.h"

namespace DBase {

/**
 * Result implementation for PSQL database - OO-wrapper for libpq
 * library
 */
class PSQLResult : public Result {
public:
	/**
	 * Implementing Result class interface see result.h
	 */
	PSQLResult(const Connection* _conn, const Query& _query,
			PGresult *_psql_result);
	~PSQLResult();
	ResultIterator* getIterator();
	unsigned getNumRows() {
		return max_row;
	}
	unsigned getNumCols() {
		return max_col;
	}

	friend class PSQLResultIterator;

private:
	/**
	 * libpq struct representing with result data
	 */
	PGresult* psql_result;
	/**
	 * Number of rows and cols in result data 
	 * (initialized in C-tor from PGresult struct)
	 */
	unsigned max_row;
	unsigned max_col;

};

class PSQLResult__ : public Result__ {
public:
	PSQLResult__(const Connection* _conn, const Query& _query, PGresult *_psql_result);
	~PSQLResult__();

	class Iterator__;
	friend class Iterator__;
	class Iterator__ : public Result__::Iterator__ {
		const PSQLResult__ *ptr;
		unsigned act_row;
		
	public:
		Iterator__(PSQLResult__* _res) : ptr(_res), act_row(0) { }
		
		value_type getValue() {
			//return std::string(PQgetvalue(ptr->psql_result, act_row, 0));
			return ptr->fetchRow(act_row);
		}
		void next() {
			++act_row;
		}
		bool isDone() {
			return act_row == ptr->max_row;
		}
	};

	Result__::Iterator begin();
	unsigned getNumRows();
	unsigned getNumCols();

private:
	const Row fetchRow(unsigned _num) const;
	
	PGresult* psql_result;
	unsigned max_row;
	unsigned max_col;

};

}

#endif /*PSQL_RESULTH_H_*/
