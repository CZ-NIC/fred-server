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

/**
 *  @file
 */

#ifndef CORBA_EPP_REQUEST_PARAMS_90798304534
#define CORBA_EPP_REQUEST_PARAMS_90798304534

#include "util/util.h"
#include "util/optional_value.h"

#include <string>

namespace Epp {

struct RequestParams
{
    std::string get_server_transaction_handle()const
    {
        return Util::make_svtrid(log_request_id.get_value_or(0));
    }

    unsigned long long session_id;
    std::string client_transaction_id;
    Optional< unsigned long long > log_request_id;
};

}

#endif
