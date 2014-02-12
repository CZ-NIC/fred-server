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
 *  check resolution operations - setting the final status and "postprocessing"
 */

#ifndef ADMIN_CONTACT_VERIFICATION_RESOLVE_CHECK_H_4574534345654
#define ADMIN_CONTACT_VERIFICATION_RESOLVE_CHECK_H_4574534345654

#include "util/db/nullable.h"
#include "util/optional_value.h"
#include "src/fredlib/opcontext.h"

namespace Admin {
    /**
     * Operation for setting the final status of check and triggering consequent actions
     */

    class resolve_check {
        private:
            std::string         check_handle_;
            std::string         status_handle_;
            Optional<long long> logd_request_id_;

            void postprocess_automatic_check(
                Fred::OperationContext& _ctx,
                const std::string& _check_handle);

            void postprocess_manual_check(
                Fred::OperationContext& _ctx,
                const std::string& _check_handle);

        public:

            /**
             * @param _check_handle Check to finalize
             * @param _status_handle Status to set
             * @param _logd_request_id Related logger request
             */
            resolve_check(
                const std::string&  _check_handle,
                const std::string&  _status_handle,
                Optional<long long> _logd_request_id);

            resolve_check& set_logd_request_id(Optional<long long> _logd_request_id);


            /**
             * Commits operation.
             * @throws Fred::ExceptionUnknownCheckHandle
             * @throws Fred::ExceptionUnknownCheckStatusHandle
             */
            void exec(Fred::OperationContext& _ctx);
    };
}

#endif // #include guard end
