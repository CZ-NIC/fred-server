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
#ifndef CORBA_CONVERSIONS_MONEY_HH_66469ED274324D5BAA1C30C215FFA51D
#define CORBA_CONVERSIONS_MONEY_HH_66469ED274324D5BAA1C30C215FFA51D

#include "src/util/types/money.hh"

#include <omniORB4/CORBA.h>

namespace CorbaConversion {
namespace Util {

Money unwrap_money_from_const_char_ptr(const char* in);

CORBA::String_var wrap_Money_to_corba_string(const Money& in);

} // namespace CorbaConversion::Util
} // namespace CorbaConversion

#endif
