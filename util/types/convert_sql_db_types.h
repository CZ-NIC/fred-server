/*  
 * Copyright (C) 2007  CZ.NIC, z.s.p.o.
 * 
 * This file is part of FRED.
 * 
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 * 
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file convert_sql_db_types.h
 *  Definitions of type conversions for user defined database helper types 
 *  from/to sql query string.
 */

#ifndef CONVERT_SQL_DB_TYPES_H_
#define CONVERT_SQL_DB_TYPES_H_

#include "convert_sql_pod.h"
#include "convert_sql_boost_datetime.h"
#include "date.h"
#include "datetime.h"
#include "id.h"

template<>
struct SqlConvert<Database::Date> {
  static Database::Date from(const std::string &_in) {
    return Database::Date(SqlConvert<date>::from(_in));
  }


  static std::string to(const Database::Date &_in) {
    return SqlConvert<date>::to(static_cast<date>(_in));
  }
};

template<>
struct SqlConvert<Database::DateTime> {
  static Database::DateTime from(const std::string &_in) {
    return Database::DateTime(SqlConvert<ptime>::from(_in));
  }


  static std::string to(const Database::DateTime &_in) {
    return SqlConvert<ptime>::to(static_cast<ptime>(_in));
  }
};

template<>
struct SqlConvert<Database::ID> {
  static Database::ID from(const std::string &_in) {
    return Database::ID(SqlConvert<Database::ID::value_type>::from(_in));
  }

  static std::string to(const Database::ID &_in) {
    return SqlConvert<Database::ID::value_type>::to(static_cast<Database::ID::value_type>(_in));
  }
};

#endif /*CONVERT_SQL_DB_TYPES_H_*/

