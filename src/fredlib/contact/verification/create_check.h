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
    /**
     * Creates new record in contact_check table with status @ref ContactCheckStatus::ENQUEUED. Has no sideeffects (e. g. no test is automatically created).
     */
    class CreateContactCheck
    {
            std::string         contact_handle_;
            std::string         testsuite_name_;
            Nullable<long long> logd_request_id_;

        public:
            struct ExceptionUnknownContactHandle : virtual Fred::OperationException {
                const char* what() const throw() {return "unknown contact handle";}
            };
            struct ExceptionUnknownTestsuiteName : virtual Fred::OperationException {
                const char* what() const throw() {return "unknown testsuite name";}
            };

            /**
             * constructor only with mandatory parameters
             * @param _contact_handle   identifies contact to be checked - current "snapshot" of historical data is used during check.
             * @param _testsuite_name   denotes set (by it's name) of tests to be run when this check is started.
             */
            CreateContactCheck(
                const std::string& _contact_handle,
                const std::string& _testsuite_name
            );
            /**
             * constructor with all available parameters including optional ones
             * @param _contact_handle   identifies contact to be checked - current "snapshot" of historical data is used during check.
             * @param _testsuite_name   denotes set (by it's name) of tests to be run when this check is started.
             * @param _logd_request_id  identifies (by id) optional log entry in logd related to this operation.
             */
            CreateContactCheck(
                const std::string&  _contact_handle,
                const std::string&  _testsuite_name,
                Optional<long long> _logd_request_id
            );

            /**
             * setter of optional logd_request_id
             * Call with another value for re-set, no need to unset first.
             */
            CreateContactCheck& set_logd_request_id(long long _logd_request_id);

            /**
             * Commits operation.
             * @return handle of created contact_check record.
             */
            std::string exec(OperationContext& ctx);
            // serialization
            friend std::ostream& operator<<(std::ostream& os, const CreateContactCheck& i);
            std::string to_string() const;
    };
}
#endif // #include guard end
