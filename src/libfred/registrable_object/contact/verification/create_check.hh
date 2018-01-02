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
 *  create contact check
 */

#ifndef CREATE_CHECK_HH_E7C18731A8B34B26BD3EE1B1DB989BAC
#define CREATE_CHECK_HH_E7C18731A8B34B26BD3EE1B1DB989BAC

#include "src/util/printable.hh"

#include "src/libfred/registrable_object/contact/verification/exceptions.hh"
#include "src/libfred/opcontext.hh"
#include "src/util/db/nullable.hh"
#include "src/util/optional_value.hh"
#include "src/util/uuid.hh"

namespace LibFred
{
    /**
     * Creates new record in contact_check table with status @ref ContactCheckStatus::ENQUEUE_REQ
     */
    class CreateContactCheck : public Util::Printable
    {
            unsigned long long              contact_id_;
            std::string                     testsuite_handle_;
            Nullable<unsigned long long>    logd_request_id_;

        public:
            /**
             * constructor only with mandatory parameters
             * @param _contact_id       identifies contact to be checked - current "snapshot" of historical data is used during check.
             * @param _testsuite_handle denotes testsuite to be run when this check is started.
             */
            CreateContactCheck(
                unsigned long long _contact_id,
                const std::string& _testsuite_handle
            );
            /**
             * constructor with all available parameters including optional ones
             * @param _contact_id       identifies contact to be checked - data realated to current history_id will be used during check.
             * @param _testsuite_handle denotes testsuite to be run when this check is started.
             * @param _logd_request_id  identifies optional log entry in logd related to this operation.
             */
            CreateContactCheck(
                unsigned long long              _contact_id,
                const std::string&              _testsuite_handle,
                Optional<unsigned long long>    _logd_request_id
            );

            /**
             * setter of optional logd_request_id
             * Call with another value for re-set, no need to unset first.
             */
            CreateContactCheck& set_logd_request_id(unsigned long long _logd_request_id);

            /**
             * Commits operation.
             * @throws LibFred::ExceptionUnknownContactId
             * @throws LibFred::ExceptionUnknownTestsuiteHandle
             * @return handle of created contact_check record.
             */
            std::string exec(OperationContext& ctx);
            // serialization
            virtual std::string to_string() const;
    };
}
#endif
