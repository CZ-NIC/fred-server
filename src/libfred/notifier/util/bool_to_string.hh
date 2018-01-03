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
 */

#ifndef BOOL_TO_STRING_HH_1BC2F69B6DD54065BF8FD24440A777C2
#define BOOL_TO_STRING_HH_1BC2F69B6DD54065BF8FD24440A777C2

#include <string>

namespace Notification {

    inline std::string to_string(bool _input) {
         return _input ? "1" : "0";
    }
}

#endif
