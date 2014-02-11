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

#include "util/printable.h"
#include "src/fredlib/contact/verification/exceptions.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

namespace Fred
{
    /**
     * Updates existing record in contact_test_result table. Has no sideeffects.
     */
    class UpdateContactTest : public Util::Printable {
            std::string           check_handle_;
            std::string           test_handle_;
            std::string           status_handle_;
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
                               const std::string&    _test_handle,
                               const std::string&    _status_handle);
            /**
             * constructor with all available parameters including optional ones
             * @param _check_handle     identifies which contact_check this test belongs to (by check's handle).
             * @param _test_name        denotes type of test (by it's name) to be run. Allowed values are in enum_contact_test.name in database.
             * @param _status_name      denotes status to be set by it's name. Allowed values are in enum_test_status.h or enum_contact_test_status.name in database.
             * @param _error_msg        optional error message (free text string) describing test state.
             */
            UpdateContactTest( const std::string&    _check_handle,
                               const std::string&    _test_handle,
                               const std::string&    _status_handle,
                               Optional<long long>   _logd_request_id,
                               Optional<std::string> _error_msg);

            /**
             * setter of optional logd_request_id
             */
            UpdateContactTest& set_logd_request_id (long long _logd_request_id);

            /**
             * setter of optional error message
             */
            UpdateContactTest& set_error_msg (const std::string& _error_msg);

            /**
             * Commits operation.
             * @throws Fred::ExceptionUnknownCheckHandle
             * @throws Fred::ExceptionUnknownTestHandle
             * @throws Fred::ExceptionUnknownCheckTestPair
             * @throws Fred::ExceptionUnknownTestStatusHandle
             */
            void exec(OperationContext& _ctx);
            // serialization
            virtual std::string to_string() const;
    };
}
#endif // #include guard end
