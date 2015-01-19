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


#include "util/idn_utils.h"

namespace Util {

    std::string::size_type get_utf8_char_len(const std::string& utf8str)
    {
        std::string::size_type len = 0;
        for (std::string::size_type i = 0; i < utf8str.size(); ++i)
        {
            //count UTF-8 single byte characters and leading bytes of multiple byte characters,
            //i.e. bytes different from continuation bytes with 10xxxxxx bit pattern
            //0xc0 is mask of bits which differentiates single byte characters, leading bytes and continuation bytes
            //0x80 is result of bitwise ANDing continuation byte with mask
            len += (utf8str.at(i) & 0xc0) != 0x80;
        }
        return len;
    }

}


