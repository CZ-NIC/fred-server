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
 *  @file
 *  update contact check
 */

#ifndef CONTACT_VERIFICATION_UPDATE_CHECK_51547983410_
#define CONTACT_VERIFICATION_UPDATE_CHECK_51547983410_

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

namespace Fred
{

    class UpdateContactCheck {
        std::string         check_handle_;          // identify which check to update
        std::string         status_name_;     // check status
        Nullable<long long> logd_request_id_; // entry in log_entry database table

        public:
            // constructors
            UpdateContactCheck( const std::string& _check_handle,
                                const std::string& _status_name);

            UpdateContactCheck( const std::string&  _check_handle,
                                const std::string&  _status_name,
                                Optional<long long> _logd_request_id
            );

            // setters for optional properties
            UpdateContactCheck& set_logd_request_id (long long _logd_request_id);
            UpdateContactCheck& unset_logd_request_id ();

            // exec and serialization
            void exec(OperationContext& _ctx);
            friend std::ostream& operator<<(std::ostream& _os, const CreateContactCheck& _i);
            std::string to_string() const;
    };
}
#endif // #include guard end
