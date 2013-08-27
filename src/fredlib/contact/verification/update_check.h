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
    /**
     * Updates existing record in contact_check table. Has no sideeffects.
     */
    class UpdateContactCheck {
            std::string         check_handle_;
            std::string         status_name_;
            Nullable<long long> logd_request_id_;

        public:
            /**
             * constructor only with mandatory parameters
             * @param _check_handle     identifies which contact_check to update.
             * @param _status_name      denotes status to be set. Allowed values are in enum_check_status.h or enum_contact_check_status in database.
             */
            UpdateContactCheck( const std::string& _check_handle,
                                const std::string& _status_name);

            /**
             * constructor with all available parameters including optional ones
             * @param _check_handle     identifies which contact_check to update by it's handle.
             * @param _status_name      denotes status to be set by it's name. Allowed values are in enum_check_status.h or enum_contact_check_status.name in database.
             * @param _logd_request_id  denotes entry in log_entry (by id) database table related to this update.
             */
            UpdateContactCheck( const std::string&  _check_handle,
                                const std::string&  _status_name,
                                Optional<long long> _logd_request_id
            );

            /**
             * setter of optional logd_request_id
             * Call with another value for re-set, no need to unset first.
             */
            UpdateContactCheck& set_logd_request_id (long long _logd_request_id);
            /**
             * unsetter of optional logd_request_id
             * Erases set value. Is idempotent.
             * If no value is set at exec() run no logd_request is reffered to by this update.
             */
            UpdateContactCheck& unset_logd_request_id ();

            /**
             * commits this operation
             */
            void exec(OperationContext& _ctx);
            // serialization
            friend std::ostream& operator<<(std::ostream& _os, const UpdateContactCheck& _i);
            std::string to_string() const;
    };
}
#endif // #include guard end
