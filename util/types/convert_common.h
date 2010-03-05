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
 *  @file convert_common.h
 */

#ifndef CONVERT_COMMON_H_
#define CONVERT_COMMON_H_

#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>

#include "base_exception.h"


class ConversionError : public ::Exception {
public:
  ConversionError(const std::string &_dir, const std::string &_class)
                : ::Exception("ConversionError: " + _dir + " through class '" + _class + "'") { }
};



template<class Iter>
void str_tolower_range(Iter beg, Iter end) {
  for (Iter it = beg; it != end; ++it) {
    *it = std::tolower(static_cast<unsigned char>(*it));
  }
}



template<class T> 
struct StreamConvertor {
  static T from(const std::string &_in) {
    std::istringstream stream(_in);
    T out;
    if (!(stream >> out)) {
      throw ConversionError("from", "StreamConvertor");
    }
    return out;
  }


  static std::string to(const T &_in) {
    std::ostringstream stream;
    if (!(stream << _in)) {
      throw ConversionError("to", "StreamConvertor");
    }
    return stream.str();
  }
};



template<class T>
struct NumericsConvertor : StreamConvertor<T> {
  static T from(const std::string &_in) {
    const int digits = std::numeric_limits<T>::digits10;
    std::istringstream stream(_in);
    T out;
    if (!(stream >> std::setprecision(digits) >> out)) {
      throw ConversionError("from", "NumericsConvertor");
    }
    return out;
  }


  static std::string to(const T &_in) {
    const int digits = std::numeric_limits<T>::digits10;
    std::ostringstream stream;
    if (!(stream << std::setprecision(digits) << _in)) {
      throw ConversionError("to", "NumericsConvertor");
    }
    return stream.str();
  }
};


#endif /*CONVERT_COMMON_H_*/

