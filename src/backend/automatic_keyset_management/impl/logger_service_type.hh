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
#ifndef LOGGER_SERVICE_TYPE_HH_F5732BF1850D4E8EA88BD89E558E00AE
#define LOGGER_SERVICE_TYPE_HH_F5732BF1850D4E8EA88BD89E558E00AE

#include <string>

namespace Fred {
namespace Backend {
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

} // namespace Fred::Backend::AutomaticKeysetManagement::Impl
} // namespace Fred::Backend::AutomaticKeysetManagement
} // namespace Fred::Backend
} // namespace Fred

#endif
