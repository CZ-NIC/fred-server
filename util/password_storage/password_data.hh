/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PASSWORD_DATA_HH_1CFF3678F5A669D20515A667F8B5BABD//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PASSWORD_DATA_HH_1CFF3678F5A669D20515A667F8B5BABD

#include "util/nonconvertible.hh"

#include <string>

namespace PasswordStorage {

struct PasswordDataTag;

typedef Util::Nonconvertible<std::string>::Named<PasswordDataTag> PasswordData;

}//namespace PasswordStorage

#endif//PASSWORD_DATA_HH_1CFF3678F5A669D20515A667F8B5BABD
