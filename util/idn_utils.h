/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  @file
 *  IDN related utils
 */


#ifndef IDN_UTIL_H_328c50f4056c45c4a284b437935ccc77
#define IDN_UTIL_H_328c50f4056c45c4a284b437935ccc77

#include <string>
#include <boost/noncopyable.hpp>

namespace Util {

/**
 *  Number of characters in UTF-8 encoded string
 *  @return number of characters in UTF8 encoded string
 */

std::string::size_type get_utf8_char_len(const std::string& utf8str);

}
#endif
