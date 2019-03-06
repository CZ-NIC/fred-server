/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file query.h
 *  Set of classes for holding queries data in structure format.
 *
 *  Note: classes interfaces are not final due to merge with old_query
 *        implemetation. Also there is a little mess with holding double
 *        impl. of basic element like Table, Column, Condition etc. because
 *        there is dependency in SelectQuery for database filtering
 *        - this should be resolved by finalizing model library.
 *        
 */


#ifndef QUERY_HH_975265E60463469487FE1B0438845E5A
#define QUERY_HH_975265E60463469487FE1B0438845E5A

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <boost/function.hpp>

#include "src/util/db/statement.hh"
#include "util/db/value.hh"
#include "src/util/db/query/query_common.hh"
#include "src/util/db/query/query_old.hh"


namespace Database {


class WhereStatement {
public:
  typedef std::string                                          operator_t;
  typedef Internal::ValueCondition                             condition_type;
  typedef std::vector<std::pair<operator_t, condition_type> >  condition_list;

  WhereStatement(const Internal::Table *_table) : table_(_table) { }


  WhereStatement clone(const Internal::Table *_table = 0) const {
    std::cout << "clone() where_statement -- table " << _table << std::endl;
    WhereStatement cloned(_table ? _table : table_->clone());

    condition_list::const_iterator it = conditions_.begin();
    for (; it != conditions_.end(); ++it) {
      cloned.add(it->second.getLeft().getName(),
                 it->second.getOperator(),
                 it->second.getRight(),
                 it->first);
    }

    std::cout << "clone() where_statement -- table " << _table << " return" << std::endl;
    return cloned;
  }

  void add(const std::string &_name,
           const condition_type::operator_t &_op,
           const condition_type::value_type &_value,
           const operator_t &_connect_with) {

    conditions_.push_back(std::make_pair(_connect_with, 
                                         condition_type(Internal::Column(_name, table_), _op, _value)));
  }


  std::string toSql(boost::function<std::string(std::string)> _esc_func) const {
    std::string tmp;

    if (!conditions_.empty()) {
      condition_list::const_iterator it = conditions_.begin();
      tmp = it->first + " " + it->second.toSql(_esc_func);
      for (++it; it != conditions_.end() && conditions_.size() > 1; ++it) {
        tmp += " " + it->first + " " + it->second.toSql(_esc_func);
      }
    }

    return tmp;
  }


  bool empty() const {
    return !conditions_.size();
  }


  void clear() {
    conditions_.clear();
  }


protected:
  const Internal::Table    *table_;
  condition_list            conditions_;
};



class QueryBase : public Statement {
public:
  typedef Internal::Table       table_type;
  typedef std::stringstream     buffer_type;
  

  explicit QueryBase(const table_type &_table) : table_(_table) { }


  virtual ~QueryBase() { }


  virtual std::string toSql(escape_function_type _esc_func) = 0;


  virtual void regenerateTableAliases(unsigned &_num) {
    std::stringstream tmp;
    tmp << table_.getName() << "_" << _num++;
    table_.setAlias(tmp.str());
  }


protected:
  table_type  table_;
};



class FilterableQuery : public QueryBase {
public:
  typedef WhereStatement where_type;


  explicit FilterableQuery(const table_type &_table) : QueryBase(_table), where_(&table_) { }


  virtual ~FilterableQuery() { }


  where_type& where() {
    return where_;
  }


protected:
  where_type where_; 
};



class DeleteQuery : public FilterableQuery {
public:
  explicit DeleteQuery(const table_type &_table) : FilterableQuery(_table) { }


  explicit DeleteQuery(const std::string &_tname) : FilterableQuery(table_type(_tname)) { }


  virtual ~DeleteQuery() { }


  std::string toSql(escape_function_type _esc_func) {
    buffer_type buffer;
    buffer << "DELETE FROM " << table_.toSql(_esc_func);
    if (!where_.empty()) {
      buffer << " WHERE 1=1 " << where_.toSql(_esc_func);
    }
    
    return buffer.str();
  }
};



class InsertQuery : public QueryBase {
public:
  explicit InsertQuery(const table_type &_table) : QueryBase(_table) { }


  explicit InsertQuery(const std::string &_tname) : QueryBase(table_type(_tname)) { }

  /**
   * HACK: old SelectQuery handling
   */
  InsertQuery(const std::string& _table, const SelectQuery& _query) : QueryBase(_table) {
    buffer_type buffer;
    buffer << "INSERT INTO " << _table << " " << _query.str();
    insert_select_ = buffer.str();
  }


  virtual ~InsertQuery() { }


  void add(const std::string &_field, const Value &_value) {
    values_.push_back(std::make_pair(_field, _value));
  }


  std::string toSql(escape_function_type _esc_func) {
    /* HACK: old SelectQuery handling */
    if (!insert_select_.empty()) {
      return insert_select_;
    }

    std::string tmp_fields, tmp_values;

    if (!values_.empty()) {
      value_list::const_iterator it = values_.begin();
      tmp_fields = it->first;
      tmp_values = it->second.toSql(_esc_func);
      for (++it; it != values_.end() && values_.size() > 1; ++it) {
        tmp_fields += ", " + it->first;
        tmp_values += ", " + it->second.toSql(_esc_func);
      }
    }

    buffer_type buffer;
    buffer << "INSERT INTO " << table_.toSql(_esc_func) << " (" << tmp_fields << ") "
           << "VALUES (" << tmp_values << ")";

    return buffer.str();
  }


  virtual bool empty() const {
    return values_.empty();
  }


protected:
  typedef std::pair<std::string, Value>    value_type;
  typedef std::vector<value_type>          value_list;

  value_list   values_;
  std::string  insert_select_;
};



class UpdateQuery : public FilterableQuery {
public:
  explicit UpdateQuery(const table_type &_table) : FilterableQuery(_table) { }


  explicit UpdateQuery(const std::string &_tname) : FilterableQuery(table_type(_tname)) { }


  virtual ~UpdateQuery() { }


  void add(const std::string &_field, const Value &_value) {
    values_.push_back(std::make_pair(_field, _value));
  }


  std::string toSql(escape_function_type _esc_func) {
    std::string tmp_set;

    if (!values_.empty()) {
      value_list::const_iterator it = values_.begin();
      tmp_set = it->first + " = " + it->second.toSql(_esc_func);
      for (++it; it != values_.end() && values_.size() > 1; ++it) {
        tmp_set += ", " + it->first + " = " + it->second.toSql(_esc_func);
      }
    }

    buffer_type buffer;
    buffer << "UPDATE " << table_.toSql(_esc_func) << " SET " << tmp_set
           << (where_.empty() ? "" : " WHERE 1=1 " + where_.toSql(_esc_func));

    if (where_.empty()) {
        throw std::runtime_error("update query with empty WHERE clause: "
                "(" + buffer.str() + ") NOT executed");
    }

    return buffer.str();
  }


  virtual bool empty() const {
    return values_.empty();
  }


protected:
  typedef std::pair<std::string, Value>       value_type;
  typedef std::vector<value_type>             value_list;

  value_list values_;  
};


}

#endif /*QUERY_H_*/

