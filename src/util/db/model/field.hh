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
/**
 *  @file field.h
 *  Wrapper for variable with specific data for
 *  database manipulation
 */

#ifndef FIELD_HH_60D5E7629E9C4269AAB249F84D3CD433
#define FIELD_HH_60D5E7629E9C4269AAB249F84D3CD433


namespace Field {


template<class _type>
class Field {
public:
  typedef _type  value_type;


  Field() : value_(), is_set_(false) { }


  Field(const value_type &_value)
      : value_(_value),
        is_set_(true) { }


  virtual ~Field() { }


  operator value_type() const {
    return value_;
  }


  Field<value_type>& operator =(const value_type &_value) {
    value_ = _value;
    is_set_ = true;
    return *this;
  }


  Field<value_type>& operator =(const Database::Value &_value) {
    if (!_value.isnull()) {
      value_ = _value;
      is_set_ = true;
    }
    else {
      is_set_ = false;
    }

    return *this;
  }


  void changed(bool _flag) {
    is_set_ = _flag;
  }


  bool isChanged() const {
    return is_set_;
  }


  const value_type& get() const {
    return value_;
  }


protected:
  value_type value_;
  bool       is_set_;
};



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


  virtual ~Field() { }


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


  Field<value_type>& operator =(const Database::Value &_value) {
    if (!_value.isnull()) {
      value_ = static_cast<std::string>(_value);
      is_set_ = true;
    }
    else {
      is_set_ = false;
    }

    return *this;
  }


  void changed(bool _flag) {
    is_set_ = _flag;
  }


  bool isChanged() const {
    return is_set_;
  }


  const value_type& get() const {
    return value_;
  }


protected:
  value_type value_;
  bool       is_set_;
};



namespace Lazy {


template<class _type>
class Field : public ::Field::Field<_type> {
public:
  typedef ::Field::Field<_type>       super;
  typedef typename super::value_type  value_type;


  virtual ~Field() {
    if (this->value_) {
      delete this->value_;
      this->value_ = 0;
    }
  }


  Field<value_type>& operator =(const value_type &_value) {
    this->value_ = _value;
    this->is_set_ = true;
    return *this;
  }
};



template<class _type, class _container = std::deque<_type*> >
class List : public _container {
public:
  typedef _container                  super;
  typedef typename super::value_type  value_type;


  List() : _container() {
  }


  virtual ~List() {
    for (typename super::iterator it = this->begin(); it != this->end(); ++it) {
      delete *it;
    }
  }
};


}

}


#endif /*FIELD_H_*/

