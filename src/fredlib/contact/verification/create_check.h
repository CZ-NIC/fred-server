/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  @file create_check.h
 *  create contact check
 */

#ifndef CONTACT_VERIFICATION_CREATE_CHECK_51537653410_
#define CONTACT_VERIFICATION_CREATE_CHECK_51537653410_

#include <boost/date_time/posix_time/posix_time.hpp>
#include "fredlib/contact/verification/postgres_types.h"

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"

namespace Fred
{

    class CreateContactCheck
    {
        Pg::Integer contact_history_id_; //id of contact to be checked - history is used so that contact data won't change during check
        Pg::Serial testsuite_id_; //id of testsuite definition
        Pg::BigSerial logd_request_id_; //id of the new entry in log_entry database table
        Pg::Serial status_id_;

    public:
        CreateContactCheck(Pg::Integer _contact_history_id, Pg::Serial _testsuite_id, Pg::BigSerial _logd_request_id, Pg::Serial _status_id);
        boost::posix_time::ptime exec(OperationContext& ctx, const std::string& returned_timestamp_pg_time_zone_name = "Europe/Prague");

        friend std::ostream& operator<<(std::ostream& os, const CreateContactCheck& i);
        std::string to_string() const;
    };
}
#endif // CONTACT_VERIFICATION_CREATE_CHECK_51537653410_
