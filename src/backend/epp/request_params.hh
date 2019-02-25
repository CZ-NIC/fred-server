/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
#ifndef REQUEST_PARAMS_HH_98806E060ED8431C9014D6E0847B513E
#define REQUEST_PARAMS_HH_98806E060ED8431C9014D6E0847B513E

#include "util/util.hh"

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
        return Util::make_svtrid(log_request_id ? *log_request_id : 0);
    }


};

} // namespace Epp

#endif
