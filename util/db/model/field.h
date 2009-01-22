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
 *  @file field.h
 *  Wrapper for variable with specific data for
 *  database manipulation
 */

#ifndef FIELD_H_
#define FIELD_H_

#include <ostream>
#include <model_list.h>

namespace Field {


template<class _type>
class Field {
public:
  typedef _type  value_type;


  Field() : value_(), is_set_(false) { }


  Field(const value_type &_value)
      : value_(_value),
        is_set_(true) { }


  ~Field() { }


  operator value_type() const {
    return value_;
  }


  Field<value_type>& operator =(const value_type &_value) {
    value_ = _value;
    is_set_ = true;
    return *this;
  }


  bool isSet() const {
    return is_set_;
  }


  const value_type& get() const {
    return value_;
  }


  value_type& get() {
    return value_;
  }


  template<class __type>
  friend std::ostream& operator<<(std::ostream &_os, const Field<__type> &_field);


private:
  value_type value_;
  bool       is_set_;
};


template<class __type>
std::ostream& operator<<(std::ostream &_os, const Field<__type> &_field) {
  return (_field.is_set_ ? _os << _field.value_ : _os << "NULL");
}


template<class __type>
std::ostream& operator<<(std::ostream &_os, const Field< ::Model::List<__type> > &_field) {
  return _os;
}


template<>
class Field<std::string> {
public:
  typedef std::string  value_type;


  Field() : is_set_(false) { }


  Field(const value_type &_value)
      : value_(_value),
        is_set_(true) { }


  Field(const char *_value)
      : value_(_value),
        is_set_(true) { }


  ~Field() { }


  operator value_type() const {
    return value_;
  }


  Field<value_type>& operator =(const value_type &_value) {
    value_ = _value;
    is_set_ = true;
    return *this;
  }


  Field<value_type>& operator =(const char *_value) {
    value_ = _value;
    is_set_ = true;
    return *this;
  }



  bool isSet() const {
    return is_set_;
  }


  const value_type& get() const {
    return value_;
  }


  friend std::ostream& operator<<(std::ostream &_os, const Field<std::string> &_field);


private:
  value_type value_;
  bool       is_set_;
};


std::ostream& operator<<(std::ostream &_os, const Field<std::string> &_field) {
  return (_field.is_set_ ? _os << _field.value_ : _os << "NULL");
}



}


#endif /*FIELD_H_*/

