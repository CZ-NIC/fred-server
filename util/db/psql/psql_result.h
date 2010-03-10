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
 *  @file psql_result.h
 *  Implementation of result object for PSQL database.
 */


#ifndef PSQL_RESULT_H_
#define PSQL_RESULT_H_

#include <libpq-fe.h>
#include <cstdlib>
#include <string>
#include <iterator>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include "../row.h"
#include "../value.h"
#include "../db_exceptions.h"


namespace Database {


/**
 * \class PSQLResult
 * \brief PSQL result object
 */
class PSQLResult {
protected:
  boost::shared_ptr<PGresult>  psql_result_; /**< wrapped result structure from lipq library */


public:
  typedef unsigned                     size_type;
  typedef Value                        value_type;
  typedef Row_<PSQLResult, value_type> Row;

  friend class Row_<PSQLResult, value_type>;
  friend class Row_<PSQLResult, value_type>::Iterator;
  friend class Result_<PSQLResult>;


  /**
   * Constructors and destructor
   */
  PSQLResult() {
  }


  PSQLResult(PGresult *_psql_result) {
    psql_result_.reset(_psql_result, PQclear);
  }


  PSQLResult(const PSQLResult &_other) {
    psql_result_ = _other.psql_result_;
  }


  virtual ~PSQLResult() {
    clear();
  }


  virtual void clear() {
  }

  /**
   * Implementation of corresponding methods called by Result_ template
   */

  size_type size() const {
    return PQntuples(psql_result_.get());
  }

  /**
   * Number of rows affected by non-select query
   */

  size_type rows_affected() const
  {
    std::string number_in_string(PQcmdTuples(psql_result_.get()));
    size_type ret = boost::lexical_cast<size_type>(number_in_string);
    return ret;
  }

  /**
   * Iterator interface definition
   */
  class Iterator;
  friend class Iterator;
  class Iterator : public std::iterator<std::bidirectional_iterator_tag, Row> {
  protected:
    const PSQLResult *data_ptr_;
    unsigned row_;

  public:
    Iterator(const PSQLResult *_data_ptr, unsigned _row = 0) : data_ptr_(_data_ptr), row_(_row) {
    }


    Iterator(const Iterator &_other) {
      data_ptr_ = _other.data_ptr_;
      row_ = _other.row_;
    }


    value_type operator*() const {
      return value_type(data_ptr_, row_);
    }


    Iterator& operator+(int _n) {
      row_ += _n;
      return *this;
    }


    Iterator& operator++() {
      ++row_;
      return *this;
    }


    Iterator& operator+=(int _n) {
      row_ += _n;
      return *this;
    }


    Iterator& operator-(int _n) {
      row_ -= _n;
      return *this;
    }


    Iterator& operator--() {
      --row_;
      return *this;
    }


    bool operator==(const Iterator& _other) const {
      return (data_ptr_ == _other.data_ptr_ && row_ == _other.row_);
    }


    bool operator!=(const Iterator& _other) const {
      return !(*this == _other);
    }

  };


  Iterator begin() const {
    return Iterator(this);
  }


  Iterator end() const {
    return Iterator(this, size());
  }


protected:
  /**
   * @return number of columns
   */
  size_type cols_() const {
    return PQnfields(psql_result_.get());
  }


  /**
   * @return number of rows
   */
  size_type rows_() const {
    return PQntuples(psql_result_.get());
  }


  /**
   * @param  _r row number
   * @param  _c column number
   * @return    value from result at position [_r, _c]
   */
  std::string value_(size_type _r, size_type _c) const /* throw(OutOfRange) */ {
    if (_r >= rows_()) {
      throw OutOfRange(0, rows_(), _r);
    }
    if (_c >= cols_()) {
      throw OutOfRange(0, cols_(), _c);
    }
    return PQgetvalue(psql_result_.get(), _r, _c);
  }


  /**
   * @param  _r row number
   * @param  _c column number
   * @return    true if value from result at position [_r, _c] is null, false otherwise
   */
  bool value_is_null_(size_type _r, size_type _c) const /* throw(OutOfRange) */{
    if (_r >= rows_()) {
      throw OutOfRange(0, rows_(), _r);
    }
    if (_c >= cols_()) {
      throw OutOfRange(0, cols_(), _c);
    }
    return PQgetisnull(psql_result_.get(), _r, _c);
  }


  /**
   * @param  _r row number
   * @param  _c column name
   * @return    value from result at position [_r, _c] 
   */
  std::string value_(size_type _r, const std::string _c) const /* throw(NoSuchField) */{
    int field = PQfnumber(psql_result_.get(), _c.c_str());
    if (field == -1) {
      throw NoSuchField(_c);
    }
    return value_(_r, field);
  }


  /**
   * @param  _r row number
   * @param  _c column name
   * @return    true if value from result at position [_r, _c] is null, false otherwise
   */
  bool value_is_null_(size_type _r, const std::string _c) const /* throw(NoSuchField) */{
    int field = PQfnumber(psql_result_.get(), _c.c_str());
    if (field == -1) {
      throw NoSuchField(_c);
    }
    return value_is_null_(_r, field);
  }
};

}

#endif

