/*
 * Copyright (C) 2012-2019  CZ.NIC, z. s. p. o.
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
 *  @common_wrappers.h
 *  common CORBA type wrappers
 */

#ifndef COMMON_WRAPPERS_HH_C6F60F2FC4074C348513E75AEEA7503F
#define COMMON_WRAPPERS_HH_C6F60F2FC4074C348513E75AEEA7503F

#include <string>
#include <boost/algorithm/string.hpp>

#include <omniORB4/CORBA.h>

inline char* corba_wrap_string(const char* _s)
{
    return CORBA::string_dup(_s);
}


inline char* corba_wrap_string(const std::string &_s)
{
    return corba_wrap_string(_s.c_str());
}

#endif
