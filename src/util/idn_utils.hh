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


#ifndef IDN_UTILS_HH_8F13274CBB504A15A42BB02ECBF45CA6
#define IDN_UTILS_HH_8F13274CBB504A15A42BB02ECBF45CA6

#include <string>
#include "src/util/optional_value.hh"

namespace Util {

/**
 *  Number of characters in UTF-8 encoded string, no validation performed on input
 *  @return number of characters in UTF8 encoded string
 */
std::string::size_type get_utf8_char_len(const std::string& utf8str);


/**
 *  Conversion of UTF-8 encoded string to punnycode, no validation performed on input
 *  @return punnycode or not set in case of conversion failure
 */
Optional<std::string> convert_utf8_to_punnycode(const std::string& utf8str);

}
#endif
