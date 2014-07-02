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

#include "util/printable.h"
#include "src/fredlib/contact/verification/exceptions.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"
#include "util/uuid.h"

namespace Fred
{
    /**
     * Updates existing record in contact_check table. Has no sideeffects.
     */
    class UpdateContactCheck : public Util::Printable {
            uuid                            check_handle_;
            std::string                     status_handle_;
            Nullable<unsigned long long>    logd_request_id_;

        public:
            /**
             * constructor only with mandatory parameters
             * @param _check_handle     identifies which contact_check to update.
             * @param _status_handle    denotes status to be set. Allowed values are in enum_check_status.h or enum_contact_check_status in database.
             */
            UpdateContactCheck( const uuid&         _check_handle,
                                const std::string&  _status_handle);

            /**
             * constructor with all available parameters including optional ones
             * @param _check_handle     identifies which contact_check to update by it's handle.
             * @param _status_handle    denotes status to be set. Allowed values are in enum_check_status.h or enum_contact_check_status.name in database.
             * @param _logd_request_id  denotes entry in log_entry database table related to this update.
             */
            UpdateContactCheck( const uuid&                     _check_handle,
                                const std::string&              _status_handle,
                                Optional<unsigned long long>    _logd_request_id);

            /**
             * setter of optional logd_request_id
             * Call with another value for re-set, no need to unset first.
             */
            UpdateContactCheck& set_logd_request_id (unsigned long long _logd_request_id);

            /**
             * Commits operation.
             * @throws Fred::ExceptionUnknownCheckHandle
             * @throws Fred::ExceptionUnknownCheckStatusHandle
             */
            void exec(OperationContext& _ctx);
            // serialization
            virtual std::string to_string() const;
    };
}
#endif // #include guard end
