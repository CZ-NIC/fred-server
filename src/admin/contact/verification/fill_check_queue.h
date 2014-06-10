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
 *  push back contact verification checks to queue up to it's at maximal length
 */

#ifndef ADMIN_CONTACT_VERIFICATION_FILL_QUEUE_H_12513457488
#define ADMIN_CONTACT_VERIFICATION_FILL_QUEUE_H_12513457488

#include <vector>
#include <string>
#include <set>

#include "util/optional_value.h"

namespace Admin {
namespace ContactVerificationQueue {

    enum allowed_contact_roles {
        owner, admin_c, tech_c
    };

    std::string to_string(allowed_contact_roles _in);
    allowed_contact_roles from_string(const std::string& _in);

    struct contact_filter {
        Optional<std::string>           country_code;
        std::set<std::string>           states;
        std::set<allowed_contact_roles> roles;
    };

    struct enqueued_check {
        std::string         handle;
        unsigned long long  contact_id;
        unsigned long long  contact_history_id;

        enqueued_check(
            const std::string&  _handle,
            unsigned long long  _contact_id,
            unsigned long long  _contact_history_id
        );
    };

    /**
     * Operation for filling the contact check queue in contact_check table
     * - new checks are of specified testsuite
     * - queue length is counted without regards to testsuite
     * - (optinally) of contacts specified by filter
     */

    class fill_check_queue {
        private:
            std::string                     testsuite_handle_;
            unsigned                        max_queue_length_;
            contact_filter                  filter_;
            Optional<unsigned long long>    logd_request_id_;

        public:
            /**
             * @param _testsuite_handle Testsuite to be used for newly enqueued checks.
             * @param _max_queue_length Will not add any check above this limit.
             */
            fill_check_queue(
                const std::string& _testsuite_handle,
                unsigned _max_queue_length);

            /**
             * setter of optional contact_filter
             * Call with another value for re-set, no need to unset first.
             */
            fill_check_queue& set_contact_filter(Optional<contact_filter> _filter);

            /**
             * setter of optional logd_request_id
             * Call with another value for re-set, no need to unset first.
             */
            fill_check_queue& set_logd_request_id(Optional<unsigned long long> _logd_request_id);

            std::vector<enqueued_check> exec();
    };
}
}


#endif // #include guard end
