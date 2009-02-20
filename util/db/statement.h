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
 *  @file statement.h
 *  Base class for database statements
 */

#ifndef STATEMENT_H_
#define STATEMENT_H_

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


  virtual std::string toSql(escape_function_type _esc_func) {
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

