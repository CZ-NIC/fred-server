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
#ifndef LOGGER_REQUEST_OBJECT_TYPE_HH_D6110E1DAB464453A3B44F61E4FAA764
#define LOGGER_REQUEST_OBJECT_TYPE_HH_D6110E1DAB464453A3B44F61E4FAA764

#include <string>

namespace LibFred {
namespace AutomaticKeysetManagement {
namespace Impl {

struct LoggerRequestObjectType
{
    enum Enum
    {
        keyset,
        domain
    };
};

template <LoggerRequestObjectType::Enum>
std::string to_fred_logger_request_object_type_name();

} // namespace LibFred::AutomaticKeysetManagement::Impl
} // namespace LibFred::AutomaticKeysetManagement
} // namespace LibFred

#endif
