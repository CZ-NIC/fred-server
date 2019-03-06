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
#include "src/backend/automatic_keyset_management/impl/logger_request_type.hh"

namespace LibFred {
namespace AutomaticKeysetManagement {
namespace Impl {

template <>
std::string to_fred_logger_request_type_name<LoggerRequestType::akm_turn_on>()
{
    return "AkmTurnOn";
}

template <>
std::string to_fred_logger_request_type_name<LoggerRequestType::akm_turn_off>()
{
    return "AkmTurnOff";
}

template <>
std::string to_fred_logger_request_type_name<LoggerRequestType::akm_rollover>()
{
    return "AkmRollover";
}

} // namespace LibFred::AutomaticKeysetManagement::Impl
} // namespace LibFred::AutomaticKeysetManagement
} // namespace LibFred

