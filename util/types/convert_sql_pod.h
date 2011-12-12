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
 *  @file convert_sql_pod.h
 *  Definitions of type conversions for POD types from/to 
 *  sql query string.
 */

#ifndef CONVERT_SQL_POD_H_
#define CONVERT_SQL_POD_H_


#include <limits>

#include "convert_sql_base.h"



template<>
struct SqlConvert<bool> {
  static bool from(const std::string &_in) {
    std::string low = _in;
    switch (_in[0]) {
      case 't':
      case 'T':
      case '1':
        if (_in.size() == 1)
          return true;

        str_tolower_range(low.begin(), low.end());
        if (low == "true")
          return true;
        
        break;

      case 'f':
      case 'F':
      case '0':
        if (_in.size() == 1) 
          return false;

        str_tolower_range(low.begin(), low.end());
        if (low == "false")
          return false;
        
        break;
    }
    throw ConversionError("from sql", "SqlConvert<bool>");
  }


  static std::string to(const bool &_in)
  {
    if(_in) return "t";
    else return "f";
  }
};



#define CONVERT_TEMPLATE_SPECIALIZATION(_type, _class) \
  template<>                                           \
  struct SqlConvert<_type> : public _class<_type> {};

CONVERT_TEMPLATE_SPECIALIZATION(short,                  NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(int,                    NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(unsigned int,           NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(long,                   NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(unsigned long,          NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(long long int,          NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(unsigned long long int, NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(float,                  NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(double,                 NumericsConvertor);

#undef CONVERT_TEMPLATE_SPECIALIZATION

#endif /*CONVERT_SQL_POD_H_*/

