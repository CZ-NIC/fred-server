/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
#include "src/bin/corba/util/corba_conversions_string.hh"

#include <stdexcept>

namespace LibFred {
namespace Corba {

CORBA::String_var wrap_string(const std::string& in)
{
    return CORBA::string_dup(in.c_str());
}


std::string unwrap_string(const char* in)
{
    std::string out;

    if (in != NULL)
    {
        out.assign(in);
    }

    return out;
}


std::string unwrap_string_from_const_char_ptr(const char* in)
{
    if (in == NULL)
    {
        throw std::runtime_error("unwrap_string_from_const_char_ptr: in is NULL");
    }
    return std::string(in);
}


CORBA::String_var wrap_const_char_ptr_to_corba_string(const char* in)
{
    if (in == NULL)
    {
        throw std::runtime_error("wrap_const_char_ptr_to_string_member: in is NULL");
    }
    return CORBA::string_dup(in);
}


CORBA::String_var wrap_string_to_corba_string(const std::string& in)
{
    return CORBA::string_dup(in.c_str());
}


} // namespace LibFred::Corba
} // namespace LibFred
