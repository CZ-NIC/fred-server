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
    /**
     * Updates existing record in contact_test_result table. Has no sideeffects.
     */
    class UpdateContactTest {
            std::string           check_handle_;
            std::string           test_name_;
            std::string           status_name_;
            Nullable<long long>   logd_request_id_;
            Nullable<std::string> error_msg_;

        public:
            /**
             * constructor only with mandatory parameters
             * @param _check_handle     identifies which contact_check this test belongs to (by check's handle).
             * @param _test_name        denotes type of test (by it's name) to be run. Allowed values are in enum_contact_test.name in database.
             * @param _status_name      denotes status to be set by it's name. Allowed values are in enum_test_status.h or enum_contact_test_status.name in database.
             */
            UpdateContactTest( const std::string&    _check_handle,
                               const std::string&    _test_name,
                               const std::string&    _status_name);
            /**
             * constructor with all available parameters including optional ones
             * @param _check_handle     identifies which contact_check this test belongs to (by check's handle).
             * @param _test_name        denotes type of test (by it's name) to be run. Allowed values are in enum_contact_test.name in database.
             * @param _status_name      denotes status to be set by it's name. Allowed values are in enum_test_status.h or enum_contact_test_status.name in database.
             * @param _error_msg        optional error message (free text string) describing test state.
             */
            UpdateContactTest( const std::string&    _check_handle,
                               const std::string&    _test_name,
                               const std::string&    _status_name,
                               Optional<long long>   _logd_request_id,
                               Optional<std::string> _error_msg);

            /**
             * setter of optional logd_request_id
             */
            UpdateContactTest& set_logd_request_id (long long _logd_request_id);
            /**
             * unsetter of optional logd_request_id
             * Erases set value. Is idempotent.
             * If no value is set at exec() run no logd_request is reffered to by this record after creation.
             */
            UpdateContactTest& unset_logd_request_id ();

            /**
             * setter of optional error message
             */
            UpdateContactTest& set_error_msg (const std::string& _error_msg);
            /**
             * unsetter of optional error message
             * Erases set value. Is idempotent.
             * If no value is set at exec() run no error message is stored for this update.
             */
            UpdateContactTest& unset_error_msg ();

            // exec and serialization
            void exec(OperationContext& _ctx);
            friend std::ostream& operator<<(std::ostream& _os, const UpdateContactTest& _i);
            std::string to_string() const;
    };
}
#endif // #include guard end
