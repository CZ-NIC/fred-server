/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
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
#include <boost/tokenizer.hpp>
#include <boost/utility.hpp>
#include "src/util/db/query/query_old.hh"
#include "util/log/logger.hh"


namespace Database {

Query::Query(const std::string& _str_query) {
  sql_buffer << _str_query;
  m_initialized = true;
}


Query::Query(const char* _str_query) {
  sql_buffer << _str_query;
  m_initialized = true;
}

std::ostream& operator<<(std::ostream &_os, const Query& _q) {
  return _os << _q.sql_buffer.str();
}


Query::~Query() {
}


SelectQuery::SelectQuery(const std::string& _cols, const std::string& _table) :
  Query() {
  select_s << _cols;
  from_s << _table;
  offset_r = 0;
  limit_r = 0;
}


SelectQuery::~SelectQuery() {
  std::for_each(select_v.begin(), select_v.end(), boost::checked_deleter<Column>());
}


void SelectQuery::finalize() {
  if (!order_by_s.str().empty()) {
    sql_buffer << " ORDER BY " << order_by_s.str();
  }

  if (offset_r > 0) {
      sql_buffer << " OFFSET " << offset_r;
  }

  if (limit_r> 0) {
    sql_buffer << " LIMIT " << limit_r;
  }
  m_initialized = false;
}


void SelectQuery::make(escape_function_type _esc_func) {
  TRACE("[CALL] SelectQuery::make()");
  if (m_initialized) {
    LOGGER.debug(boost::format("buffer-generated select SQL = %1%") % sql_buffer.str());
    return;
  }

  Query::clear();
  if (!select_v.empty()) {
    select_s.clear();
    select_s.str("");
    for (std::vector<Column*>::const_iterator it = select_v.begin(); it
    != select_v.end(); ++it) {
      select_s << (select_s.str().empty() ? "" : ", ") << (*it)->str();
    }
  }

  if (m_where_prepared_values.size() > 0) {
    boost::format where_prepared_format(m_where_prepared_string.str());

    for (std::vector<Database::Value>::iterator it = m_where_prepared_values.begin();
         it != m_where_prepared_values.end();
         ++it) {
      where_prepared_format % (*it).toSql(_esc_func);
    }
    where_s.clear();
    where_s.str("");
    where_s << where_prepared_format.str();
  }
  else {
    where_s << m_where_prepared_string.str();
  }

  sql_buffer << "SELECT " << select_s.str() << " FROM " << from_s.str();
  if (!where_s.str().empty())
  sql_buffer << " WHERE 1=1 " << where_s.str();
  if (!group_by_s.str().empty())
  sql_buffer << " GROUP BY " << group_by_s.str(); 
  finalize();
  LOGGER.debug(boost::format("generated select SQL = %1%") % sql_buffer.str());
}


void SelectQuery::join(const std::string& _cols, const std::string& _tables,
const std::string _cond) {
  select_s << (select_s.str().empty() ? " " : ", ") << _cols;
  from_s << (from_s.str().empty() ? " " : ", ") << _tables;
  where_s << " AND " << _cond;
  m_initialized = false;
}


void SelectQuery::addSelect(const std::string& _cols, Table& _table) {
  boost::char_separator<char> sep(" ");
  boost::tokenizer<boost::char_separator<char> > tok(_cols, sep);
  for (boost::tokenizer<boost::char_separator<char> >::iterator it =
  tok.begin(); it != tok.end(); ++it) {
    select_v.push_back(new Column(*it, _table));
  }
  m_initialized = false;
}


void SelectQuery::addSelect(Column* _c) {
  select_v.push_back(_c);
  m_initialized = false;
}


void SelectQuery::clear() {
  Query::clear();
  select_s.str("");
  select_s.clear();
  from_s.clear();
  from_s.str("");
  where_s.clear();
  where_s.str("");
  group_by_s.clear();
  group_by_s.str("");
  order_by_s.clear();
  order_by_s.str("");
  offset_r = 0;
  limit_r = 0;
  std::for_each(select_v.begin(),select_v.end(),boost::checked_deleter<Column>());
  select_v.clear();
  
  m_where_prepared_string.clear();
  m_where_prepared_string.str("");
  m_where_prepared_values.clear();
  m_initialized = false;
}


// InsertQuery::InsertQuery(const std::string& _table, const SelectQuery& _sq){
//   sql_buffer << "INSERT INTO " << _table << " " << _sq.str();
//   m_initialized = true;
// }
// 
// 
// InsertQuery::InsertQuery(const std::string& _table) : table_(_table) {
//   m_initialized = false;
// }
// 
// 
// void InsertQuery::add(const std::string& _column, const Value& _value) {
//   values_.push_back(std::make_pair<std::string, Value>(_column, _value));
// }
// 
// 
// void InsertQuery::make() {
//   if (m_initialized) 
//     return;
//   
//   if (table_.empty()) {
//     m_initialized = true;
//     return;
//   }
//   else {
//     if (values_.empty()) {
//       return;
//     }
//     sql_buffer << "INSERT INTO " << table_;
//     
//     ValueContainer::const_iterator it = values_.begin();
//     std::stringstream columns_part;
//     std::stringstream values_part;
//     
//     columns_part << "(" << it->first;
//     values_part << "VALUES (" << it->second;
//     for (++it; it != values_.end(); ++it) {
//       std::stringstream value;
//       value << it->second;
// 
//       columns_part << ", " << it->first;
//       values_part  << ", " << (it->second.quoted() ? "E'" : "") 
//                    << (it->second.quoted() ? Util::escape(value.str()) : value.str())
//                    << (it->second.quoted() ? "'" : "");
//     }
//     columns_part << ")";
//     values_part  << ")";
//     
//     sql_buffer << " " << columns_part.str() << " " << values_part.str();
//   }
// }

}
