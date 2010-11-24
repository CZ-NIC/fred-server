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
 *  @file convert_str_boost_datetime.h
 *  Definitions of type conversions for boost datetime types from/to string.
 */

#ifndef CONVERT_STR_BOOST_DATETIME_H_
#define CONVERT_STR_BOOST_DATETIME_H_

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "convert_str_base.h"
#include "base_exception.h"

using namespace boost::posix_time;
using namespace boost::gregorian;


template<>
struct StrConvert<date> {
public:
  static date from(const std::string &_in) {
    try {
      return from_string(_in);
    }
    catch (...) {
      throw ConversionError("from", "StrConvert<date>");
    }
  }


  static std::string to(const date &_in) {
    try {
      if (_in.is_special()) return "";
      std::ostringstream stream;
      stream.imbue(std::locale(stream.getloc(), new date_facet("%d.%m.%Y")));
      stream << _in;
      return stream.str();

    }
    catch (...) {
      throw ConversionError("to", "StrConvert<date>");
    }
  }
};



template<>
struct StrConvert<ptime> {
public:
  static ptime from(const std::string &_in) {
    try {
      return time_from_string(_in);
    }
    catch (...) {
      throw ConversionError("from", "StrConvert<ptime>");
    }
  }


  static std::string to(const ptime &_in) {
    try {
      std::ostringstream stream;
      stream.imbue(std::locale(stream.getloc(), new time_facet("%d.%m.%Y %H:%M:%S")));
      stream << _in;
      return stream.str();
    }
    catch (...) {
      throw ConversionError("to", "StrConvert<ptime>");
    }
  }
};


#endif /*CONVERT_STR_BOOST_DATETIME_H_*/

