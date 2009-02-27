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
 *  @file sqlize.h
 *  Interface for convert structures for convert from/to sql
 */

#ifndef SQLIZE_H_
#define SQLIZE_H_

#include "convert_sql_base.h"


/**
 * converts value to format for sql query
 */
template<class T>
inline std::string sqlize(const T &_value) {
  return SqlConvert<T>::to(_value);
}



/**
 * converts value from database stored format to value type
 */
template<class T>
inline T unsqlize(const std::string &_value) {
  return SqlConvert<T>::from(_value);
}


#endif /*STRINGIFY_H_*/

