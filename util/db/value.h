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

#include "types/data_types.h"
#include "log/logger.h"


namespace Database {


/**
 * Macros for defining constructors, cast operators and assignments operatror
 * from other types to string.
 *
 * Can be use only for simple conversions - calling function or method on
 * appropriate type. Special handling must be done manually.
 */
#define CONSTRUCTOR_BY_CONV_METHOD(_type, _method, _quoted)     \
Value(const _type& _value) : is_null_(false),                   \
                             value_(_value._method()),          \
                             quoted_output_(_quoted) {          \
}


#define CAST_OPERATOR_BY_CONV_METHOD(_type, _method)            \
operator _type() const {                                        \
  _type cast;                                                   \
  cast._method(value_);                                         \
  return cast;                                                  \
}


#define ASSIGN_OPERATOR_BY_CONV_METHOD(_type, _method, _quoted) \
Value& operator =(const _type &_value) {                        \
  value_ = _value._method();                                    \
  is_null_ = false;                                             \
  quoted_output_ = _quoted;                                     \
  return *this;                                                 \
}


#define CONSTRUCTOR_BY_CONV_FUNCT(_type, _funct, _quoted)       \
Value(const _type& _value) : is_null_(false),                   \
                             value_(_funct(_value)),            \
                             quoted_output_(_quoted) {          \
}


#define CAST_OPERATOR_BY_CONV_FUNCT(_type, _funct)              \
operator _type() const {                                        \
  return _funct(value_);                                        \
}


#define ASSIGN_OPERATOR_BY_CONV_FUNCT(_type, _funct, _quoted)   \
Value& operator =(const _type &_value) {                        \
  value_ = _funct(_value);                                      \
  is_null_ = false;                                             \
  quoted_output_ = _quoted;                                     \
  return *this;                                                 \
}


#define HANDLE_TYPE_METHOD(_type, _to_str_fun, _from_str_fun, _quoted) \
CONSTRUCTOR_BY_CONV_METHOD(_type, _to_str_fun, _quoted)                \
CAST_OPERATOR_BY_CONV_METHOD(_type, _from_str_fun)                     \
ASSIGN_OPERATOR_BY_CONV_METHOD(_type, _to_str_fun, _quoted)


#define HANDLE_TYPE_FUNCT(_type, _to_str_fun, _from_str_fun, _quoted)  \
CONSTRUCTOR_BY_CONV_FUNCT(_type, _to_str_fun, _quoted)                 \
CAST_OPERATOR_BY_CONV_FUNCT(_type, _from_str_fun)                      \
ASSIGN_OPERATOR_BY_CONV_FUNCT(_type, _to_str_fun, _quoted)


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
  Value() : is_null_(true),
            value_("NULL"),
            quoted_output_(false) {
  }


  /**
   * construct and conversion operator for string values
   */
  Value(const std::string& _value) : is_null_(false),
                                     value_(_value),
                                     quoted_output_(true) {
  }


  Value(const char* _value) : is_null_(false),
                              value_(_value),
                              quoted_output_(true) {
  }


  operator std::string() const {
    if (is_null_)
      return "NULL";
    else
      return value_;
  }


  Value& operator =(const std::string &_value) {
    value_ = _value;
    is_null_ = false;
    quoted_output_ = true;
    return *this;
  }

  /**
   * Database::ID need special handling in constructor
   */
  Value(const Database::ID& _value) : is_null_(false),
                                      value_(_value.to_string()),
                                      quoted_output_(false) {
    /* do extra checking for NULL (zero value is not valid ID) */
    if (_value == 0)
      is_null_ = true;
  }


  CAST_OPERATOR_BY_CONV_METHOD(Database::ID, from_string)


  /**
   * boost::ptime need special handling
   */
  Value(const ptime &_value) : is_null_(false),
                               value_(to_iso_extended_string(_value)),
                               quoted_output_(true) {
    if (_value.is_special()) {
      is_null_ = true;
      quoted_output_ = false;
    }
  }


  CAST_OPERATOR_BY_CONV_FUNCT(ptime, time_from_string)


  Value& operator =(const ptime &_value) {
    if (_value.is_special()) {
      is_null_ = true;
      quoted_output_ = false;
    }
    else {
      value_ = to_iso_extended_string(_value);
      is_null_ = false;
      quoted_output_ = true;
    }
    return *this;
  }


  /**
   * boost::date need special handling
   */
  Value(const date &_value) : is_null_(false),
                              value_(to_iso_extended_string(_value)),
                              quoted_output_(true) {
    if (_value.is_special()) {
      is_null_ = true;
      quoted_output_ = false;
    }
  }


  CAST_OPERATOR_BY_CONV_FUNCT(date, boost::gregorian::from_string)


  Value& operator =(const date &_value) {
    if (_value.is_special()) {
      is_null_ = true;
      quoted_output_ = false;
    }
    else {
      value_ = to_iso_extended_string(_value);
      is_null_ = false;
      quoted_output_ = true;
    }
    return *this;
  }


  /**
   * definitions of construtors and conversion operators to POD types
   * uses Conversion class
   */
  HANDLE_TYPE_FUNCT(short, Conversion<short>::to_string, Conversion<short>::from_string, false)
  HANDLE_TYPE_FUNCT(int, Conversion<int>::to_string, Conversion<int>::from_string, false)
  HANDLE_TYPE_FUNCT(long, Conversion<long>::to_string, Conversion<long>::from_string, false)
  HANDLE_TYPE_FUNCT(long long, Conversion<long long>::to_string, Conversion<long long>::from_string, false)
  HANDLE_TYPE_FUNCT(unsigned, Conversion<unsigned>::to_string, Conversion<unsigned>::from_string, false)
  HANDLE_TYPE_FUNCT(unsigned long, Conversion<unsigned long>::to_string, Conversion<unsigned long>::from_string, false)
  HANDLE_TYPE_FUNCT(unsigned long long, Conversion<unsigned long long>::to_string, Conversion<unsigned long long>::from_string, false)
  HANDLE_TYPE_FUNCT(bool, Conversion<bool>::to_string, Conversion<bool>::from_string, true)


  /**
   * definition of constructors and conversion operator for user defined types
   * uses Conversion class
   */
  HANDLE_TYPE_METHOD(DateTime, to_string, from_string, true)
  HANDLE_TYPE_METHOD(Date, to_string, from_string, true)
  HANDLE_TYPE_METHOD(Money, to_string, from_string, true)


  /* assigment */
  Value& operator=(const Value &_other) {
    is_null_ = _other.is_null_;
    value_ = _other.value_;
    quoted_output_ = _other.quoted_output_;
    return *this;
  }


  /* value output operator */
  friend std::ostream& operator<<(std::ostream& _os, const Value& _value);

  /**
   * @return  flag if this value should be quoted in SQL statement or not
   */
  bool quoted() const {
    return quoted_output_;
  }


  const std::string str() const {
    return value_;
  }

  bool isnull() const {
	  return (is_null_ == true);
  }

  bool operator !() const {
	  return isnull();
  }

protected:
  bool is_null_;        /**< flag if value is NULL */
  std::string value_;   /**< value in std::string representation */
  bool quoted_output_;  /**< SQL value quotation flag */
};


inline std::ostream& operator<<(std::ostream& _os, const Value& _value) {
  return (_value.is_null_ ? _os << "NULL" : _os <<  _value.value_);
}


}

#endif /*VALUE_H_*/
