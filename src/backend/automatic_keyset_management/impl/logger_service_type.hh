/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef LOGGER_SERVICE_TYPE_HH_F5732BF1850D4E8EA88BD89E558E00AE
#define LOGGER_SERVICE_TYPE_HH_F5732BF1850D4E8EA88BD89E558E00AE

#include <string>

namespace LibFred {
namespace AutomaticKeysetManagement {
namespace Impl {

struct LoggerServiceType
{
    enum Enum
    {
        admin
    };
};

template <LoggerServiceType::Enum>
std::string to_fred_logger_service_type_name();

template <>
std::string to_fred_logger_service_type_name<LoggerServiceType::admin>()
{
    return "Admin";
}

} // namespace LibFred::AutomaticKeysetManagement::Impl
} // namespace LibFred::AutomaticKeysetManagement
} // namespace LibFred

#endif