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
 *  @file stringify.h
 *  Interface for convert structures for convert from/to string
 *  (inspired by C++ FAQ Lite)
 */

#ifndef STRINGIFY_HH_6E69CB02AB5C43C6B97D9A72E7979116
#define STRINGIFY_HH_6E69CB02AB5C43C6B97D9A72E7979116

#include "src/util/types/convert_str_base.hh"
#include "src/util/types/convert_str_pod.hh"
#include "src/util/types/convert_str_boost_datetime.hh"


template<class T>
inline std::string stringify(const T &_value) {
  return StrConvert<T>::to(_value);
}



template<class T>
inline T unstringify(const std::string &_value) {
  return StrConvert<T>::from(_value);
}


#endif /*STRINGIFY_H_*/

