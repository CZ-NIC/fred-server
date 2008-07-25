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

#ifndef VALUE_HPP_
#define VALUE_HPP_

#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "types/data_types.h"


namespace Database {

#define CONSTRUCTOR_FROM_DEFINED_TYPE(_type, _funct, _quoted)   \
Value(const _type& _value) : is_null_(false),                   \
                             value_(_value._funct()),           \
                             quoted_output_(_quoted) {          \
}

#define CAST_OPERATOR_FROM_DEFINED_TYPE(_type, _funct)          \
operator _type() const {                                        \
  _type cast;                                                   \
  cast._funct(value_);                                          \
  return cast;                                                  \
}

#define CONSTRUCTOR_FROM_POD_TYPE(_type, _funct, _quoted)       \
Value(const _type& _value) : is_null_(false),                   \
                             value_(_funct(_value)),            \
                             quoted_output_(_quoted) {          \
}

#define CAST_OPERATOR_FROM_POD_TYPE(_type, _funct)              \
operator _type() const {                                        \
  return _funct(value_);                                        \
}


#define HANDLE_TYPE(_type, _to_str_fun, _from_str_fun, _quoted) \
CONSTRUCTOR_FROM_DEFINED_TYPE(_type, _to_str_fun, _quoted)      \
CAST_OPERATOR_FROM_DEFINED_TYPE(_type, _from_str_fun)

#define HANDLE_POD_TYPE(_type, _to_str, _from_str, _quoted)     \
CONSTRUCTOR_FROM_POD_TYPE(_type, _to_str, _quoted)              \
CAST_OPERATOR_FROM_POD_TYPE(_type, _from_str)

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
  typedef std::string value_type;

  /**
   * default constructor for NULL value
   */
  Value() : is_null_(true),
            value_("NULL"),
            quoted_output_(false) {
  }


//  Value(const char* _value) : is_null_(false),
//                              value_(_value),
//                              quoted_output_(false) {
//  }


  /**
   * construct and conversion operator for value_type
   */
  Value(const value_type& _value) : is_null_(false),
                                    value_(_value),
                                    quoted_output_(true) /* check if changing 'value_type' */ {
  }
  operator value_type() const {
    return value_;
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
  CAST_OPERATOR_FROM_DEFINED_TYPE(Database::ID, from_string)
// HANDLE_TYPE(ID, to_string, from_string, false)
  

  /**
   * definitions of construtors and conversion operators to POD types
   * uses Conversion class
   */
  HANDLE_POD_TYPE(short, Conversion<short>::to_string, Conversion<short>::from_string, false)
  HANDLE_POD_TYPE(int, Conversion<int>::to_string, Conversion<int>::from_string, false)
  HANDLE_POD_TYPE(long, Conversion<long>::to_string, Conversion<long>::from_string, false)
  HANDLE_POD_TYPE(long long, Conversion<long long>::to_string, Conversion<long long>::from_string, false)
  HANDLE_POD_TYPE(unsigned, Conversion<unsigned>::to_string, Conversion<unsigned>::from_string, false)
  HANDLE_POD_TYPE(unsigned long, Conversion<unsigned long>::to_string, Conversion<unsigned long>::from_string, false)
  HANDLE_POD_TYPE(unsigned long long, Conversion<unsigned long long>::to_string, Conversion<unsigned long long>::from_string, false)
  HANDLE_POD_TYPE(bool, Conversion<bool>::to_string, Conversion<bool>::from_string, true)

  /**
   * definition of constructors and conversion operator for user defined types
   * uses Conversion class
   */
  HANDLE_TYPE(DateTime, to_string, from_string, true)
  HANDLE_TYPE(Date, to_string, from_string, true)
  HANDLE_TYPE(Money, to_string, from_string, true)


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

protected:
  bool is_null_;        /**< flag if value is NULL */
  value_type value_;    /**< value in value_type representation */
  bool quoted_output_;  /**< SQL value quotation flag */
};


inline std::ostream& operator<<(std::ostream& _os, const Value& _value) {
  return (_value.is_null_ ? _os << "NULL" : _os <<  _value.value_);
}


/**
 * TODO: overload stream for query itself (not use .buffer()) and make
 *       new operator here using quoted_output_ in it
 *
 *       something like:
 *
 *       inline Query& operator<<(Query& _q, const Value& _Value) {
 *         return _q << (_value.quoted_output_ ? "'" : "") << _value.value_ << (_value.quoted_output_ ? "'" : "");
 *       }
 */

}

#endif /*VALUE_HPP_*/
