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
 *  @file value.h
 *  Implementation of value object.
 */

#ifndef VALUE_H_
#define VALUE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "util/util.h"
#include "log/logger.h"
#include "types/data_types.h"
#include "types/convert_sql_pod.h"
#include "types/convert_sql_boost_datetime.h"
#include "types/convert_sql_db_types.h"
#include "types/sqlize.h"
#include <boost/function.hpp>


namespace Database {


/**
 * Macros for defining constructors, cast operators and assignments operator
 * from other types to string.
 *
 * Can be use only for simple conversions - calling function or method on
 * appropriate type. Special handling must be done manually.
 */
#define CONSTRUCTOR(_type, _quoted, _escaped)                \
Value(const _type& _value) : is_null_(false),                \
                             value_(sqlize(_value)),         \
                             quoted_output_(_quoted),        \
                             escaped_output_(_escaped)       \
{                                                            \
}


#define ASSIGN_OPERATOR(_type, _quoted, _escaped)            \
Value& operator =(const _type &_value) {                     \
  value_ = sqlize(_value);                                   \
  is_null_ = false;                                          \
  quoted_output_ = _quoted;                                  \
  escaped_output_ = _escaped;                                \
  return *this;                                              \
}


#define CAST_OPERATOR(_type, _init)                          \
operator _type() const {                                     \
  return is_null_ ? _init : unsqlize<_type>(value_);         \
}


#define HANDLE_TYPE(_type, _init, _quoted, _escaped)         \
CONSTRUCTOR(_type, _quoted, _escaped)                        \
ASSIGN_OPERATOR(_type, _quoted, _escaped)                    \
CAST_OPERATOR(_type, _init)



/**
 * \class Value
 * \brief Value object is return type from Database::Row container
 *
 * It contains all needed contructors and conversion operators
 * for supported types to simplify getting/setting data from/to SQL
 * result/query
 */
class Value {
public:
  /**
   * default constructor for NULL value
   */
  Value() 
      : is_null_(true),
        value_(),
        quoted_output_(false),
        escaped_output_(false) {
  }


  /**
   * constructor for database result object
   */
  Value(const std::string &_value, bool _is_null) 
      : is_null_(_is_null),
        value_(_value),
        quoted_output_(_is_null ? false : true),
        escaped_output_(quoted_output_) {
  }


  /**
   * full parameter constructor
   */
  Value(const std::string &_value,
        bool _is_null,
        bool _quoted,
        bool _escaped)
      : is_null_(_is_null),
        value_(_value),
        quoted_output_(_quoted),
        escaped_output_(_escaped) {
  }
        

  /**
   * construct and conversion operator for string values
   */
  Value(const std::string& _value)
      : is_null_(false),
        value_(_value),
        quoted_output_(true),
        escaped_output_(true) {
  }


  Value(const char* _value) 
      : is_null_(false),
        value_(_value),
        quoted_output_(true),
        escaped_output_(true) {
  }


  operator std::string() const {
    return (is_null_ ? "" : value_);
  }


  Value& operator =(const std::string &_value) {
    value_ = _value;
    is_null_ = false;
    quoted_output_ = true;
    escaped_output_ = true;
    return *this;
  }


  /**
   * Database::ID need special handling
   */
  Value(const Database::ID& _value) 
      : is_null_(false),
        value_(sqlize(_value)),
        quoted_output_(false),
        escaped_output_(false) {
    /* do extra checking for NULL (zero value is not valid ID) */
    if (_value == 0)
      is_null_ = true;
  }


  CAST_OPERATOR(Database::ID, Database::ID())


  /**
   * boost::ptime need special handling
   */
  Value(const ptime &_value) {
    __init_date_time(_value);
  }


  CAST_OPERATOR(ptime, ptime())


  Value& operator =(const ptime &_value) {
    __init_date_time(_value);
    return *this;
  }


  /**
   * boost::date need special handling
   */
  Value(const date &_value) {
    __init_date_time(_value);
  }


  CAST_OPERATOR(date, date())


  Value& operator =(const date &_value) {
    __init_date_time(_value);
    return *this;
  }


  /**
   * DateTime need special handling
   */
  Value(const DateTime &_value) {
    __init_date_time(_value);
  }


  CAST_OPERATOR(DateTime, DateTime())


  Value& operator =(const DateTime &_value) {
    __init_date_time(_value);
    return *this;
  }


  /**
   * Date need special handling
   */
  Value(const Date &_value) {
    __init_date_time(_value);
  }


  CAST_OPERATOR(Date, Date())


  Value& operator =(const Date &_value) {
    __init_date_time(_value);
    return *this;
  }



  HANDLE_TYPE(short,              0,          false, false)
  HANDLE_TYPE(int,                0,          false, false)
  HANDLE_TYPE(long,               0,          false, false)
  HANDLE_TYPE(long long,          0,          false, false)
  HANDLE_TYPE(unsigned,           0,          false, false)
  HANDLE_TYPE(unsigned long,      0,          false, false)
  HANDLE_TYPE(unsigned long long, 0,          false, false)
  HANDLE_TYPE(float,              0,          false, false)
  HANDLE_TYPE(bool,               0,          true,  false)

//  HANDLE_TYPE(DateTime,           DateTime(), true,  false)
//  HANDLE_TYPE(Date,               Date(),     true,  false)



  /* assigment */
  Value& operator=(const Value &_other) {
    is_null_ = _other.is_null_;
    value_ = _other.value_;
    quoted_output_ = _other.quoted_output_;
    escaped_output_ = _other.escaped_output_;
    return *this;
  }


  /**
   * @return  flag if this value should be quoted in SQL statement or not
   */
  bool quoted() const {
    return quoted_output_;
  }


  const std::string str() const {
    return value_;
  }


  /**
   * @return  whether is value null or not
   */
  bool isnull() const {
	  return is_null_;
  }


  bool operator !() const {
	  return isnull();
  }


  /**
   * to sql string serialization
   *
   * @param _esc_func  string escape function pointer
   */
  std::string toSql(boost::function<std::string(std::string)> _esc_func) const {
    if (is_null_) {
      return "NULL";
    }
    else {
      return (quoted_output_ ? (escaped_output_ ? "E'" : "'") + _esc_func(value_) + "'" : value_);
    }
  }


  /* value output operator */
  friend std::ostream& operator<<(std::ostream& _os, const Value& _value);


private:
  /**
   * common initialization code for boost ptime
   * and boost date types
   */
  template<class T>
  void __init_date_time(const T _value) {
    escaped_output_ = false;
    if (_value.is_special()) {
      is_null_ = true;
      quoted_output_ = false;
    }
    else {
      value_ = sqlize(_value);
      is_null_ = false;
      quoted_output_ = true;
    }
  }


protected:
  bool        is_null_;        /**< flag if value is NULL */
  std::string value_;          /**< value in std::string representation */
  bool        quoted_output_;  /**< SQL value quotation flag */
  bool        escaped_output_; /**< SQL value escape flag */
};


inline std::ostream& operator<<(std::ostream& _os, const Value& _value) {
  return (_value.is_null_ ? _os << "NULL" : _os << _value.toSql(&Util::escape2));
}



std::vector<std::string> array_to_vector(std::string _dbarr);


}

#endif /*VALUE_H_*/
