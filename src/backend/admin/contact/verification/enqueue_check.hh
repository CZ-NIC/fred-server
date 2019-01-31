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

#ifndef ENQUEUE_CHECK_HH_59E11A20502C4483846A7AC376D5E36C
#define ENQUEUE_CHECK_HH_59E11A20502C4483846A7AC376D5E36C

#include "src/backend/admin/contact/verification/exceptions.hh"

#include "libfred/registrable_object/contact/verification/create_check.hh"

#include "libfred/opcontext.hh"
#include "util/optional_value.hh"

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

/**
 * Creates new check for specified contact with specified testsuite with status ENQUEUE_REQ
 *
 * @param _contact_id specifies contact to be checked
 * @param _testsuite_handle specifice testsuite of enqueued check
 *
 * @returns handle of check
 *
 * @throws Admin::ExceptionUnknownContactId
 * @throws Admin::ExceptionUnknownTestsuiteHandle
 */
std::string request_check_enqueueing(
        LibFred::OperationContext& _ctx,
        unsigned long long _contact_id,
        const std::string& _testsuite_handle,
        Optional<unsigned long long> _logd_request_id = Optional<unsigned long long>());


/**
 * Updates check with status ENQUEUE_REQ to status ENQUEUED
 *
 * @param _check_handle specifies check to be updated
 *
 * @throws Admin::ExceptionUnknownCheckHandle
 * @throws Admin::ExceptionCheckNotUpdateable
 */
void confirm_check_enqueueing(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle,
        Optional<unsigned long long> _logd_request_id = Optional<unsigned long long>());


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
 * @throws Admin::ExceptionUnknownContactId
 * @throws Admin::ExceptionUnknownTestsuiteHandle
 */
std::string enqueue_check(
        LibFred::OperationContext& _ctx,
        unsigned long long _contact_id,
        const std::string& _testsuite_handle,
        Optional<unsigned long long> _logd_request_id = Optional<unsigned long long>());


/**
 * Creates new check for specified contact with specified testsuite
 * in case no other check (regardless of testsuite) for the same contact exists (not only same history of contact).
 *
 * @param _contact_id specifies contact to be checked
 * @param _testsuite_handle specifice testsuite of enqueued check
 *
 * @returns handle of enqueued check (if any)
 *
 * @throws Admin::ExceptionUnknownContactId
 * @throws Admin::ExceptionUnknownTestsuiteHandle
 */
Optional<std::string> enqueue_check_if_no_other_exists(
        LibFred::OperationContext& _ctx,
        unsigned long long _contact_id,
        const std::string& _testsuite_handle,
        Optional<unsigned long long> _logd_request_id = Optional<unsigned long long>());


} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif
