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
 *  @file statement.h
 *  Base class for database statements
 */

#ifndef STATEMENT_HH_F74F6399A5C64CAC9D86782D909CF546
#define STATEMENT_HH_F74F6399A5C64CAC9D86782D909CF546

#include <boost/utility.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <string>


namespace Database {


class Statement {
public:
  typedef std::string                                buffer_type;
  typedef boost::function<std::string(std::string)>  escape_function_type;

  
  Statement() {
  }


  Statement(const std::string &_stmt) : buffer_(_stmt) {
  }


  Statement(const Statement &_other) {
    buffer_ = _other.buffer_;
  }


  Statement& operator=(const Statement &_stmt) {
    buffer_ = _stmt.buffer_;
    return *this;
  }


  Statement& operator=(const std::string &_stmt) {
    buffer_ = _stmt;
    return *this;
  }


  virtual ~Statement() {
  }


  virtual std::string toSql(escape_function_type) {
    return buffer_;
  }


  virtual const std::string str() const {
    return buffer_;
  }


  virtual bool empty() const {
    return buffer_.empty();
  }


protected:
  buffer_type  buffer_;
};


}


#endif /*STATEMENT_H_*/

