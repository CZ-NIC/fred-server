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
 *  implementation for CORBA conversion
 */

#include "util/corba_conversion.h"

namespace CorbaConversion
{

IntegralConversionOutOfRange::IntegralConversionOutOfRange()
:   std::invalid_argument("Converted value is out of range")
{
}

IntegralConversionOutOfRange::IntegralConversionOutOfRange(const std::string &msg)
:   std::invalid_argument(msg)
{
}

CORBA::String_var wrap_string(const std::string &src)
{
    return src.c_str();
}

}
