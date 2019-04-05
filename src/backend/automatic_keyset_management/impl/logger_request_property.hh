/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#ifndef LOGGER_REQUEST_PROPERTY_HH_C64338CBA51A4C709D48C664D930BA53
#define LOGGER_REQUEST_PROPERTY_HH_C64338CBA51A4C709D48C664D930BA53

#include <string>

namespace Fred {
namespace Backend {
namespace AutomaticKeysetManagement {
namespace Impl {

struct LoggerRequestProperty
{
    static const bool is_child = true;

    enum Enum
    {
        name,
        keyset,
        old_dns_key,
        new_dns_key,
        op_tr_id
    };
};

template <LoggerRequestProperty::Enum>
std::string to_fred_logger_request_property_name();

} // namespace Fred::Backend::AutomaticKeysetManagement::Impl
} // namespace Fred::Backend::AutomaticKeysetManagement
} // namespace Fred::Backend
} // namespace Fred

#endif
