/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef REQUEST_PARAMS_H_E846A60680374E6287EA3DE17D3A5FEA
#define REQUEST_PARAMS_H_E846A60680374E6287EA3DE17D3A5FEA

#include "util/util.h"

#include <boost/optional.hpp>

#include <string>

namespace Epp {

struct RequestParams
{
    unsigned long long session_id;
    std::string client_transaction_id;
    boost::optional<unsigned long long> log_request_id;

    std::string get_server_transaction_handle() const
    {
        return Util::make_svtrid(log_request_id ? log_request_id.value() : 0);
    }


};

} // namespace Epp

#endif
