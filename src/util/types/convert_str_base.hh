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
 *  @file convert_str_base.h
 *  Definitions of type conversions from/to string.
 *  (inspired by C++ FAQ Lite)
 */


#ifndef CONVERT_STR_BASE_HH_81612106A8714C109F48C29DF8AC1463
#define CONVERT_STR_BASE_HH_81612106A8714C109F48C29DF8AC1463

#include <iostream>
#include <sstream>
#include <iomanip>

#include "util/types/convert_common.hh"


/**
 * fallback convert to string, if no specialization
 * found convert by stream convertor
 */
template<class T>
struct StrConvert : StreamConvertor<T> {};


#endif /*CONVERT_STR_BASE_H_*/

