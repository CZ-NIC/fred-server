#ifndef QUERY_OLD_H_
#define QUERY_OLD_H_

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <stdio.h>
#include <algorithm>
#include <cstring>
#include <boost/utility.hpp>
#include <boost/format.hpp>

#include "util/util.h"
#include "sql_helper_objects.h"
#include "../statement.h"
#include "../value.h"


namespace Database {

/*
 * Base Query class
 */
class Query : public Statement, public boost::noncopyable {
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


	virtual void clear() {
		sql_buffer.clear();
		sql_buffer.str("");
	}


	virtual void make(escape_function_type _esc_func = &Util::escape2) {
    finalize();
  };


  virtual std::string toSql(escape_function_type _esc_func) {
    make(_esc_func);
    return str();
  }


protected:
	std::stringstream sql_buffer;
	bool m_initialized;
};



class SelectQuery : public Query {
public:
	SelectQuery() :
			Query(), offset_r(0), limit_r(0) {
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

        void offset(unsigned _n) {
                offset_r = _n;
        }

	void limit(unsigned _n) {
	  /*
	   *  HACK: + 1 added to get to know if load limit value is used or not
	   *  it should be propably at the callee side
	   */
		limit_r = _n + 1;
	}
	

	void clear();


  void make(escape_function_type _esc_func = &Util::escape2);


	void finalize();


	void addSelect(const std::string& _cols, Table &_table);


	void addSelect(Column* _c);


	void join(const std::string& _cols,
            const std::string& tables,
	      		const std::string _cond);


  typedef std::stringstream             prepared_values_string;
  typedef std::vector<Database::Value>  prepared_values_storage;


	prepared_values_string& where_prepared_string() {
		return m_where_prepared_string;
	}


	prepared_values_storage& where_prepared_values() {
		m_initialized = false;
		return m_where_prepared_values;
	}


protected:
	std::stringstream         select_s;
	std::stringstream         from_s;
	std::stringstream         where_s;
	std::stringstream         group_by_s;
	std::stringstream         order_by_s;
        unsigned                  offset_r;
	unsigned                  limit_r;
	std::vector<Column*>      select_v;
	prepared_values_string    m_where_prepared_string;
	prepared_values_storage   m_where_prepared_values;
};


}
#endif /*QUERY_H_*/
