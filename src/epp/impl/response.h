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

#ifndef RESPONSE_H_40B4B7E37BC94D97BF077CA2AB6F5915
#define RESPONSE_H_40B4B7E37BC94D97BF077CA2AB6F5915

#include "src/epp/impl/exception.h"

namespace Epp {

struct Response
{
    enum Enum
    {
        ok,
        no_mesg,
        ack_mesg,
        logout,
        failed,
        exception,
        object_not_exist,
        object_exist,
        authentication_error_server_closing_connection,
        authentication_error,
        authorization_error,
        authorization_information_error,
        parameter_value_policy_error,
        status_prohibits_operation,
        object_association_prohibits_operation,
        not_eligible_for_renew,
        not_eligible_for_transfer,
        parameter_value_syntax_error,
        parameter_value_range_error,
        parameter_missing,
        billing_failure,
        max_session_limit,
    };
};

//db table enum_error
inline unsigned to_description_db_id(Response::Enum state)
{
    switch (state)
    {
        //Command completed successfully
        case Response::ok:                           return 1000;
        //Command completed successfully; no messages
        case Response::no_mesg:                      return 1300;
        //Command completed successfully; ack to dequeue
        case Response::ack_mesg:                     return 1301;
        //Command completed successfully; ending session
        case Response::logout:                       return 1500;
        //Unknown command
        case Response::exception:                    return 2000;
        //Required parameter missing
        case Response::parameter_missing:            return 2003;
        //Parameter value range error
        case Response::parameter_value_range_error:  return 2004;
        //Parameter value syntax error
        case Response::parameter_value_syntax_error: return 2005;
        //Billing failure
        case Response::billing_failure:              return 2104;
        //Object is not eligible for renewal
        case Response::not_eligible_for_renew:       return 2105;
        //Object is not eligible for transfer
        case Response::not_eligible_for_transfer:    return 2106;
        //Authentication error
        case Response::authentication_error:         return 2200;
        //Authorization error
        case Response::authorization_error:          return 2201;
        //Authorization information error
        case Response::authorization_information_error: return 2202;
        //Object exists
        case Response::object_exist:                 return 2302;
        //Object does not exist
        case Response::object_not_exist:             return 2303;
        //Object status prohibits operation
        case Response::status_prohibits_operation:   return 2304;
        //Object association prohibits operation
        case Response::object_association_prohibits_operation:         return 2305;
        //Parameter value policy error
        case Response::parameter_value_policy_error: return 2306;
        //Command failed
        case Response::failed:                       return 2400;
        //Authentication error; server closing connection
        case Response::authentication_error_server_closing_connection: return 2501;
        //Session limit exceeded; server closing connection
        case Response::max_session_limit:            return 2502;
    }
    throw InvalidResponseValue();
}

}

#endif
