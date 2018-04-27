/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef PUBLIC_REQUEST_PARAMS_HH_D11401DE20AD4684A355DD9612889EBA
#define PUBLIC_REQUEST_PARAMS_HH_D11401DE20AD4684A355DD9612889EBA

#include <vector>
#include <string>

struct ProcessPublicRequestsArgs
{
    std::vector<std::string> types;

    ProcessPublicRequestsArgs() = default;

    ProcessPublicRequestsArgs(const std::vector<std::string>&  _public_requests_types)
        : types(_public_requests_types)
    {
    }
};

#endif
