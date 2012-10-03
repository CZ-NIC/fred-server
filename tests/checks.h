/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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

#ifndef CHECKS_H_
#define CHECKS_H_

#include <boost/test/unit_test.hpp>

#include "fredlib/db_settings.h"
#include "util/types/optional.h"
#include "fredlib/contact_verification/contact.h"
#include "mojeid/public_request_verification_impl.h"

    ///check public request on contact, report args of the check, return public request id
    static unsigned long long check_public_request_on_contact(Fred::Contact::Verification::Contact& fcvc
            , const std::string & public_request_name
            , unsigned public_request_status)
    {
        //get db connection
        Database::Connection conn = Database::Manager::acquire();

        Database::Result res_public_request= conn.exec_params(
           "select pr.id from object_registry obr "
            " join public_request_objects_map prom on obr.id = prom.object_id "
            " join public_request pr on pr.id=prom.request_id "
            " join enum_public_request_type eprt on pr.request_type = eprt.id "
           " where obr.name = $1::text and eprt.name = $2::text and pr.status = $3::integer "
           , Database::query_param_list(fcvc.handle)(public_request_name)(public_request_status));
        BOOST_CHECK_MESSAGE(res_public_request.size() == 1
            , "check_public_request_on_contact public request: " << public_request_name
            << " status: " << public_request_status << " contact handle: " << fcvc.handle);

        unsigned long long public_request_id = 0;
        if (res_public_request.size() == 1) public_request_id = res_public_request[0][0];
        return public_request_id;
    }


#endif // CHECKS_H_
