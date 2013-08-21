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
 *  update contact test
 */

#ifndef CONTACT_VERIFICATION_UPDATE_TEST_51798341011_
#define CONTACT_VERIFICATION_UPDATE_TEST_51798341011_

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

namespace Fred
{

    class UpdateContactTest {
        std::string           check_handle_;    // identification is done by pair(check_handle, test_name)
        std::string           test_name_;
        std::string           status_name_;     // news status to be applied
        Nullable<long long>   logd_request_id_; // entry in log_entry database table
        Nullable<std::string> error_msg_; // entry in log_entry database table

        public:
            // constructors
            UpdateContactTest( const std::string&    _check_handle,
                               const std::string&    _test_name,
                               const std::string&    _status_name);

            UpdateContactTest( const std::string&    _check_handle,
                               const std::string&    _test_name,
                               const std::string&    _status_name,
                               Optional<long long>   _logd_request_id,
                               Optional<std::string> _error_msg);

            // setters for optional properties
            UpdateContactTest& set_logd_request_id (long long _logd_request_id);
            UpdateContactTest& unset_logd_request_id ();

            UpdateContactTest& set_error_msg (const std::string& _error_msg);
            UpdateContactTest& unset_error_msg ();

            // exec and serialization
            void exec(OperationContext& _ctx);
            friend std::ostream& operator<<(std::ostream& _os, const CreateContactCheck& _i);
            std::string to_string() const;
    };
}
#endif // #include guard end
