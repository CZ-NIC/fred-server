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
 *  @file convert_str_pod.h
 *  Definitions of type conversions for POD types from/to string.
 */

#ifndef CONVERT_STR_POD_H_
#define CONVERT_STR_POD_H_

#include <limits>

#include "convert_str_base.h"
#include "base_exception.h"


#define CONVERT_TEMPLATE_SPECIALIZATION(_type, _class) \
  template<>                                           \
  struct StrConvert<_type> : public _class<_type> {};

CONVERT_TEMPLATE_SPECIALIZATION(short,                   NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(int,                     NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(unsigned int,            NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(long,                    NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(unsigned long,           NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(long long int,           NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(unsigned long long int,  NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(float,                   NumericsConvertor);
CONVERT_TEMPLATE_SPECIALIZATION(double,                  NumericsConvertor);

#undef CONVERT_TEMPLATE_SPECIALIZATION

#endif /*CONVERT_STR_POD_H_*/

