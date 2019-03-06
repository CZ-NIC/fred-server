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
/**
 *  @file
 *  misc utils related to contact
 */

#ifndef UTIL_HH_8801447BBF01407EA488B90620A42F69
#define UTIL_HH_8801447BBF01407EA488B90620A42F69

#include <string>
#include <utility>


namespace LibFred
{
namespace ContactUtil
{
    struct ExceptionUnknownContactHistoryId {};

    std::pair<std::string, unsigned long long> contact_hid_to_handle_id_pair(unsigned long long hid);
}
}

#endif
