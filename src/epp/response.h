/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

#ifndef EPP_RESPONSE_H_654630456451
#define EPP_RESPONSE_H_654630456451

#include "src/epp/exception.h"

namespace Epp {

struct Response {
    enum Enum {
        ok                          = 1000,
        no_mesg                     = 1300,
        ack_mesg                    = 1301,
        logout                      = 1500,

        failed                      = 2400,
        exception                   = 2000,
        object_not_exist            = 2303,
        object_exist                = 2302,
        authentication_error_server_closing_connection = 2501,
        authentication_error        = 2200,
        autor_error                 = 2201,
        parametr_value_policy_error = 2306,
        status_prohibits_operation  = 2304,
        prohibits_operation         = 2305,
        not_eligible_for_renew      = 2105,
        not_eligible_for_transfer   = 2106,
        parametr_error              = 2005,
        parametr_range_error        = 2004,
        parametr_missing            = 2003,
        billing_failure             = 2104,
        max_session_limit           = 2502
    };
};

inline unsigned to_description_db_id(const Response::Enum state) {
    return static_cast<unsigned>(state);
}

template<typename T> inline typename T::Enum from_description_db_id(const unsigned id);

/**
 * @throws UnknownLocalizedDescriptionId
 */
template<> inline Response::Enum from_description_db_id<Response>(const unsigned id) {

    /* Not using simple static_cast because id value validation would look similar to switch below. */
    switch(id) {
        case 1000: return Response::ok;
        case 1300: return Response::no_mesg;
        case 1301: return Response::ack_mesg;
        case 1500: return Response::logout;

        case 2400: return Response::failed;
        case 2000: return Response::exception;
        case 2303: return Response::object_not_exist;
        case 2302: return Response::object_exist;
        case 2501: return Response::authentication_error_server_closing_connection;
        case 2200: return Response::authentication_error;
        case 2201: return Response::autor_error;
        case 2306: return Response::parametr_value_policy_error;
        case 2304: return Response::status_prohibits_operation;
        case 2305: return Response::prohibits_operation;
        case 2105: return Response::not_eligible_for_renew;
        case 2106: return Response::not_eligible_for_transfer;
        case 2005: return Response::parametr_error;
        case 2004: return Response::parametr_range_error;
        case 2003: return Response::parametr_missing;
        case 2104: return Response::billing_failure;
        case 2502: return Response::max_session_limit;
    }

    throw UnknownLocalizedDescriptionId();
}

}

#endif
