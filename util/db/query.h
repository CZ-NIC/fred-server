#ifndef QUERY_H_
#define QUERY_H_

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <stdio.h>
#include <algorithm>
#include <cstring>
#include <boost/utility.hpp>
#include <boost/format.hpp>

#include "sql_helper_objects.h"
#include "value.h"

namespace Database {

/*
 * Base Query class
 */
class Query {
public:
	Query() :
		m_initialized(false) {
	}
	Query(const std::string& _str_query);
	Query(const char* _str_query);
	Query(const Query& _q);
	virtual ~Query();
	const std::string str() const {
		return sql_buffer.str();
	}
	const char* c_str() {
		char *tmp = new char[sql_buffer.str().size() + 1];
		strcpy(tmp, sql_buffer.str().c_str());
		return tmp;
	}
	std::stringstream& buffer() {
		m_initialized = true;
		return sql_buffer;
	}

	friend std::ostream& operator<<(std::ostream &_os, const Query& _q);

	virtual bool initialized() const {
		return m_initialized;
	}

	virtual void finalize() {
		m_initialized = true;
	}

	virtual void make() {
		finalize();
	}
	
	virtual void clear() {
		sql_buffer.clear();
		sql_buffer.str("");
	}

protected:
	std::stringstream sql_buffer;
	bool m_initialized;
};

class SelectQuery : public Query {
public:
	SelectQuery() :
			Query(), limit_r(0) {
	}
	SelectQuery(const std::string& _cols, const std::string& _table);
	~SelectQuery();
	std::stringstream& select() {
		return select_s;
	}
	std::stringstream& from() {
		return from_s;
	}
	std::stringstream& where() {
		return where_s;
	}
	std::stringstream& group_by() {
		return group_by_s;
	}
	std::stringstream& order_by() {
		return order_by_s;
	}
	void limit(unsigned _n) {
	  /*
	   *  HACK: + 1 added to get to know if load limit value is used or not
	   *  it should be propably at the callee side
	   */
		limit_r = _n + 1;
	}
	
	void clear();
	void make();
	void finalize();
	void addSelect(const std::string& _cols, Table &_table);
	void addSelect(Column* _c);
	void join(const std::string& _cols, const std::string& tables,
			const std::string _cond);
	
	std::stringstream& where_prepared_string() {
		return m_where_prepared_string;
	}
	std::vector<std::string>& where_prepared_values() {
		m_initialized = false;
		return m_where_prepared_values;
	}

	const std::string debug() {
		make();
		return sql_buffer.str();
	}

protected:
	std::stringstream select_s;
	std::stringstream from_s;
	std::stringstream where_s;
	std::stringstream group_by_s;
	std::stringstream order_by_s;
	unsigned limit_r;
	std::vector<Column*> select_v;
	std::stringstream m_where_prepared_string;
	std::vector<std::string> m_where_prepared_values;
};

class InsertQuery : public Query {
protected:
  typedef std::vector<std::pair<std::string, Value> > ValueContainer;
  std::string table_;
  ValueContainer values_;
  
public:
	InsertQuery() :
		Query() {
	}
	InsertQuery(const std::string& _table, const SelectQuery& _sq);
	InsertQuery(const std::string& _table);
	void add(const std::string& _column, const Value& _value);
	virtual void make();
};

}
#endif /*QUERY_H_*/
