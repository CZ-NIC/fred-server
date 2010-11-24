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
 *  @file stringify.h
 *  Interface for convert structures for convert from/to string
 *  (inspired by C++ FAQ Lite)
 */

#ifndef STRINGIFY_H_
#define STRINGIFY_H_

#include "convert_str_base.h"
#include "convert_str_pod.h"
#include "convert_str_boost_datetime.h"


template<class T>
inline std::string stringify(const T &_value) {
  return StrConvert<T>::to(_value);
}



template<class T>
inline T unstringify(const std::string &_value) {
  return StrConvert<T>::from(_value);
}


#endif /*STRINGIFY_H_*/

