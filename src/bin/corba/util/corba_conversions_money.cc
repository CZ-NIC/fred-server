/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#include "src/bin/corba/util/corba_conversions_money.hh"

#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/util/types/money.hh"

namespace CorbaConversion {
namespace Util {

Money unwrap_money_from_const_char_ptr(const char* in)
{
    return Money(LibFred::Corba::unwrap_string_from_const_char_ptr(in));
}

CORBA::String_var wrap_Money_to_corba_string(const Money& in)
{
    return CORBA::string_dup(in.get_string().c_str());
}

} // namespace CorbaConversion::Util
} // namespace CorbaConversion
