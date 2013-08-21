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

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

namespace Fred
{

    class CreateContactCheck
    {
        std::string         contact_handle_;    // contact to be checked - current version of historical data is used during check
        std::string         testsuite_name_;    // testsuite definition
        Nullable<long long> logd_request_id_;   // entry in log_entry database table

    public:
        // constructors
        CreateContactCheck(
            const std::string& _contact_handle,
            const std::string& _testsuite_name,
        );
        CreateContactCheck(
            const std::string&  _contact_handle,
            const std::string&  _testsuite_name,
            Optional<long long> _logd_request_id
        );

        // setters for optional parameters
        CreateContactCheck& set_logd_request_id(long long _logd_request_id);
        CreateContactCheck& unset_logd_request_id();

        // exec and serialization
        /// @return handle of created contact_check
        std::string exec(OperationContext& ctx);
        friend std::ostream& operator<<(std::ostream& os, const CreateContactCheck& i);
        std::string to_string() const;
    };
}
#endif // #include guard end
