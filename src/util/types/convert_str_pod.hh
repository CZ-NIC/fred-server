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
 *  @file convert_str_pod.h
 *  Definitions of type conversions for POD types from/to string.
 */

#ifndef CONVERT_STR_POD_HH_9402ED4E44CA4BBFA3297D58E7AD959F
#define CONVERT_STR_POD_HH_9402ED4E44CA4BBFA3297D58E7AD959F

#include <limits>

#include "src/util/types/convert_str_base.hh"
#include "util/base_exception.hh"


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

