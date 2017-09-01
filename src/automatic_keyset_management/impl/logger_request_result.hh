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

#ifndef LOGGER_REQUEST_RESULT_HH_46A47DBE91924E16A56AC3457F8C7245
#define LOGGER_REQUEST_RESULT_HH_46A47DBE91924E16A56AC3457F8C7245

#include <string>

namespace Fred {
namespace AutomaticKeysetManagement {
namespace Impl {

struct LoggerRequestResult
{
    enum Enum
    {
        success,
        fail,
        error
    };
};

template <LoggerRequestResult::Enum>
std::string to_fred_logger_request_result_name();

template <>
std::string to_fred_logger_request_result_name<LoggerRequestResult::success>()
{
    return "Success";
}

template <>
std::string to_fred_logger_request_result_name<LoggerRequestResult::fail>()
{
    return "Fail";
}

template <>
std::string to_fred_logger_request_result_name<LoggerRequestResult::error>()
{
    return "Error";
}

} // namespace Fred::AutomaticKeysetManagement::Impl
} // namespace Fred::AutomaticKeysetManagement
} // namespace Fred

#endif
