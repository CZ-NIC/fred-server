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

#ifndef LOGGER_REQUEST_TYPE_HH_F7FF064A8DA940609C9DEE6F0B9254C8
#define LOGGER_REQUEST_TYPE_HH_F7FF064A8DA940609C9DEE6F0B9254C8

#include <string>

namespace Fred {
namespace AutomaticKeysetManagement {

struct LoggerRequestType
{
    enum Enum
    {
        akm_turn_on, ///< turn on automatic keyset management
        akm_rollover, ///< roll over the keys in automatic keyset management
        akm_turn_off, ///< turn off automatic keyset management
    };
};

template <LoggerRequestType::Enum>
std::string to_fred_logger_request_type_name();

} // namespace Fred::AutomaticKeysetManagement
} // namespace Fred

#endif
