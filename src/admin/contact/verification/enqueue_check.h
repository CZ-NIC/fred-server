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
 *  enqueue specified check (and invalidates checks obsoleted by it)
 */

#ifndef ADMIN_CONTACT_VERIFICATION_ENQUEUE_CHECK_H_32421015464
#define ADMIN_CONTACT_VERIFICATION_ENQUEUE_CHECK_H_32421015464

#include "src/admin/contact/verification/exceptions.h"

#include "src/fredlib/contact/verification/create_check.h"

#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"

namespace Admin {

    /**
     * Creates new check for specified contact with specified testsuite
     * and resolve all ENQUEUED checks for the same contact (not only same history of contact)
     * with status INVALIDATED.
     *
     * @param _contact_id specifies contact to be checked
     * @param _testsuite_handle specifice testsuite of enqueued check
     *
     * @returns handle of enqueued check
     *
     * @throws Fred::ExceptionUnknownContactId
     * @throws Fred::ExceptionUnknownTestsuiteHandle
     */
    std::string enqueue_check(
        Fred::OperationContext&         _ctx,
        unsigned long long              _contact_id,
        const std::string&              _testsuite_handle,
        Optional<unsigned long long>    _logd_request_id = Optional<unsigned long long>());

    /**
     * Creates new check for specified contact with specified testsuite
     * in case no other check (regardless of testsuite) for the same contact exists (not only same history of contact).
     *
     * @param _contact_id specifies contact to be checked
     * @param _testsuite_handle specifice testsuite of enqueued check
     *
     * @returns handle of enqueued check (if any)
     *
     * @throws Fred::ExceptionUnknownContactId
     * @throws Fred::ExceptionUnknownTestsuiteHandle
     */
    Optional<std::string> enqueue_check_if_no_other_exists(
        Fred::OperationContext&         _ctx,
        unsigned long long              _contact_id,
        const std::string&              _testsuite_handle,
        Optional<unsigned long long>    _logd_request_id = Optional<unsigned long long>());
}


#endif // #include guard end
