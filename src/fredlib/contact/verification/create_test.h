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
 *  \file
 *  create contact test
 */

#ifndef CONTACT_VERIFICATION_CREATE_TEST_51547658410_
#define CONTACT_VERIFICATION_CREATE_TEST_51547658410_

#include "util/printable.h"

#include "src/fredlib/contact/verification/exceptions.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

namespace Fred
{
    /**
     * Creates new record in contact_test_resutl table with status @ref ContactTestStatus::RUNNING . Has no sideeffects.
     */
    class CreateContactTest : public Util::Printable {
        std::string         check_handle_;
        std::string         test_handle_;
        Nullable<long long> logd_request_id_;

    public:
        /**
         * constructor only with mandatory parameters
         * @param _check_handle     identifies which contact_check this test belongs to (by check's handle).
         * @param _test_name        denotes type of test (by it's handle) to be run. Allowed values are in enum_contact_test.name in database.
         */
        CreateContactTest(
            const std::string& _check_handle,
            const std::string& _test_handle
        );
        /**
         * constructor with all available parameters including optional ones
         * @param _check_handle     identifies which contact_check this test belongs to (by check's handle).
         * @param _test_name        denotes type of test (by it's handle) to be run. Allowed values are in enum_contact_test.name in database.
         * @param _logd_request_id  identifies (by id) optional log entry in logd related to this operation.
         */
        CreateContactTest(
            const std::string&  _check_handle,
            const std::string&  _test_handle,
            Optional<long long> _logd_request_id
        );

        /**
         * setter of optional logd_request_id
         * Call with another value for re-set, no need to unset first.
         */
        CreateContactTest& set_logd_request_id(long long _logd_request_id);

        /**
         * Commits operation.
         * @throws Fred::ExceptionUnknownCheckHandle
         * @throws Fred::ExceptionUnknownTestHandle
         * @throws Fred::ExceptionTestNotInMyTestsuite
         * @throws Fred::ExceptionCheckTestPairAlreadyExists
         */
        void exec(OperationContext& ctx);
        // serialization
        virtual std::string to_string() const;
    };
}
#endif // #include guard end
