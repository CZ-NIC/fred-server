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
 *  Implementation of result object for Mock database.
 */

#ifndef MOCK_RESULT_HH_E9DD2ED0D55A432AB6A5D530F930C483
#define MOCK_RESULT_HH_E9DD2ED0D55A432AB6A5D530F930C483

#include "src/deprecated/libfred/db_settings.hh"
#include "util/db/row.hh"
#include "util/db/value.hh"
#include "util/db/db_exceptions.hh"

//mock #include <libpq-fe.h>
#include <cstdlib>
#include <string>
#include <iterator>
#include <memory>

namespace Database {

struct DummyResult //PGresult replacement
{};

/**
 * \class MockResult
 * \brief Mock result object
 */
class MockResult
{
public:
  typedef unsigned                     size_type;
  typedef Value                        value_type;
  typedef Row_<MockResult, value_type> Row;

  friend class Row_<MockResult, value_type>;
  friend class Row_<MockResult, value_type>::Iterator;
  friend class Result_<MockResult>;

  /**
   * Constructors and destructor
   */
  MockResult() { }

  MockResult(DummyResult*) { }
//mock psql_result_.reset(_psql_result, PQclear);

  MockResult(const MockResult& _other)
    : psql_result_(_other.psql_result_)
  { }

  ~MockResult()
  {
    clear();
  }

  void clear() { }

  /**
   * Implementation of corresponding methods called by Result_ template
   */

  size_type size() const
  {
    return 0;//mock PQntuples(psql_result_.get());
  }

  /**
   * Iterator interface definition
   */
  class Iterator : public std::iterator<std::bidirectional_iterator_tag, Row>
  {
  public:
    Iterator(const MockResult* _data_ptr, unsigned _row = 0)
      : data_ptr_(_data_ptr),
        row_(_row)
    { }

    Iterator(const Iterator& _other)
    {
      data_ptr_ = _other.data_ptr_;
      row_ = _other.row_;
    }

    value_type operator*() const
    {
      return value_type(data_ptr_, row_);
    }

    Iterator& operator+(int _n)
    {
      row_ += _n;
      return *this;
    }

    Iterator& operator++()
    {
      ++row_;
      return *this;
    }

    Iterator& operator+=(int _n)
    {
      row_ += _n;
      return *this;
    }

    Iterator& operator-(int _n)
    {
      row_ -= _n;
      return *this;
    }

    Iterator& operator--()
    {
      --row_;
      return *this;
    }

    bool operator==(const Iterator& _other) const
    {
      return (data_ptr_ == _other.data_ptr_ && row_ == _other.row_);
    }

    bool operator!=(const Iterator& _other) const
    {
      return !(*this == _other);
    }
  private:
    const MockResult* data_ptr_;
    unsigned row_;
  };

  Iterator begin() const {
    return Iterator(this);
  }

  Iterator end() const {
    return Iterator(this, size());
  }
private:
  /**
   * @return number of columns
   */
  size_type cols_() const {
    return 0;//mock PQnfields(psql_result_.get());
  }

  /**
   * @return number of rows
   */
  size_type rows_() const {
    return 0;//mockPQntuples(psql_result_.get());
  }

  /**
   * @param  _r row number
   * @param  _c column number
   * @return    value from result at position [_r, _c]
   */
  std::string value_(size_type _r, size_type _c) const /* throw(OutOfRange) */
  {
    if (rows_() <= _r)
    {
      throw OutOfRange(_r, { 0, rows_() });
    }
    if (cols_() <= _c)
    {
      throw OutOfRange(_c, { 0, cols_() });
    }
    return std::string("");//mock PQgetvalue(psql_result_.get(), _r, _c);
  }

  /**
   * @param  _r row number
   * @param  _c column number
   * @return    true if value from result at position [_r, _c] is null, false otherwise
   */
  bool value_is_null_(size_type _r, size_type _c) const /* throw(OutOfRange) */
  {
    if (rows_() <= _r)
    {
      throw OutOfRange(_r, { 0, rows_() });
    }
    if (cols_() <= _c)
    {
      throw OutOfRange(_c, { 0, cols_() });
    }
    return true;//mock PQgetisnull(psql_result_.get(), _r, _c);
  }

  /**
   * @param  _r row number
   * @param  _c column name
   * @return    value from result at position [_r, _c]
   */
  std::string value_(size_type _r, const std::string _c) const /* throw(NoSuchField) */{
	  /*mock
    int field = PQfnumber(psql_result_.get(), _c.c_str());
    if (field == -1) {
      throw NoSuchField(_c);
    }
    */
    return std::string("");//mock value_(_r, field);
  }

  /**
   * @param  _r row number
   * @param  _c column name
   * @return    true if value from result at position [_r, _c] is null, false otherwise
   */
  bool value_is_null_(size_type _r, const std::string _c) const /* throw(NoSuchField) */{
	  /*mock
    int field = PQfnumber(psql_result_.get(), _c.c_str());
    if (field == -1) {
      throw NoSuchField(_c);
    }
    */
    return true;//mock value_is_null_(_r, field);
  }

  std::shared_ptr<DummyResult> psql_result_; /**< wrapped result structure from lipq library */
};

}//namespace Database

#endif//MOCK_RESULT_HH_E9DD2ED0D55A432AB6A5D530F930C483
