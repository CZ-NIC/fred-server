/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file
 *  check resolution operations - setting the final status and "postprocessing"
 */

#ifndef RESOLVE_CHECK_HH_E67802A4D0154696B19BFB1228955BED
#define RESOLVE_CHECK_HH_E67802A4D0154696B19BFB1228955BED

#include "src/backend/admin/contact/verification/exceptions.hh"

#include "libfred/opcontext.hh"
#include "util/db/nullable.hh"
#include "util/optional_value.hh"
#include "src/util/tz/get_psql_handle_of.hh"
#include "src/util/tz/utc.hh"

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

/**
 * Operation for setting the final status of check and triggering consequent actions
 */

class resolve_check
{
private:
    uuid check_handle_;
    std::string status_handle_;
    Optional<unsigned long long> logd_request_id_;

    static void postprocess_automatic_check(
            LibFred::OperationContext& _ctx,
            const uuid& _check_handle);


    static void postprocess_manual_check(
            LibFred::OperationContext& _ctx,
            const uuid& _check_handle);


    static void postprocess_thank_you_check(
            LibFred::OperationContext& _ctx,
            const uuid& _check_handle);


public:
    /**
     * @param _check_handle Check to finalize
     * @param _status_handle Status to set
     * @param _logd_request_id Related logger request
     */
    resolve_check(
            const uuid& _check_handle,
            const std::string& _status_handle,
            Optional<unsigned long long> _logd_request_id);


    /**
     * setter of optional logd_request_id
     * Call with another value for re-set, no need to unset first.
     */
    resolve_check& set_logd_request_id(Optional<unsigned long long> _logd_request_id);


    /**
     * Commits operation.
     * @throws Admin::ExceptionUnknownCheckHandle
     * @throws Admin::ExceptionUnknownCheckStatusHandle
     * @throws Admin::ExceptionCheckNotUpdateable
     */
    void exec(LibFred::OperationContext& _ctx, const std::string& _output_timezone = Tz::get_psql_handle_of<Tz::UTC>());


};

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif
