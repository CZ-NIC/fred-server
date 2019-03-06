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
#include "src/backend/admin/contact/verification/contact_states/delete_all.hh"
#include "src/backend/admin/contact/verification/contact_states/enum.hh"
#include "src/backend/admin/contact/verification/related_records.hh"
#include "src/backend/admin/contact/verification/resolve_check.hh"
#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/verification/enum_check_status.hh"
#include "src/deprecated/libfred/registrable_object/contact/verification/enum_testsuite_handle.hh"
#include "libfred/registrable_object/contact/verification/info_check.hh"
#include "libfred/registrable_object/contact/verification/update_check.hh"

#include "util/log/context.hh"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include <set>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {


resolve_check::resolve_check(
        const uuid& _check_handle,
        const std::string& _status_handle,
        Optional<unsigned long long> _logd_request_id)
    : check_handle_(_check_handle),
      status_handle_(_status_handle),
      logd_request_id_(_logd_request_id)
{
}


resolve_check& resolve_check::set_logd_request_id(Optional<unsigned long long> _logd_request_id)
{
    logd_request_id_ = _logd_request_id;

    return *this;
}


void resolve_check::exec(LibFred::OperationContext& _ctx, const std::string& _output_timezone)
{
    Logging::Context log("resolve_check::exec");

    // if current check status is not valid for resolution...
    {
        std::vector<std::string> allowed_statuses = LibFred::ContactCheckStatus::get_resolution_awaiting();

        if (
            std::find(
                    allowed_statuses.begin(),
                    allowed_statuses.end(),
                    LibFred::InfoContactCheck(check_handle_).exec(_ctx, _output_timezone)
                    .check_state_history.rbegin()->status_handle) == allowed_statuses.end()
            )
        {
            throw ExceptionCheckNotUpdateable();
        }
    }

    // if given new check status is not valid for resolution...
    {
        std::vector<std::string> allowed_statuses = LibFred::ContactCheckStatus::get_possible_resolutions();
        if (
            std::find(
                    allowed_statuses.begin(),
                    allowed_statuses.end(),
                    status_handle_) == allowed_statuses.end()
            )
        {
            throw ExceptionCheckNotUpdateable();
        }
    }

    try
    {
        LibFred::UpdateContactCheck(
                check_handle_,
                status_handle_,
                logd_request_id_).exec(_ctx);

        LibFred::InfoContactCheckOutput check_info = LibFred::InfoContactCheck(check_handle_).exec(_ctx);

        if (check_info.testsuite_handle == LibFred::TestsuiteHandle::AUTOMATIC)
        {
            postprocess_automatic_check(
                    _ctx,
                    check_handle_);
        }
        else if (check_info.testsuite_handle == LibFred::TestsuiteHandle::MANUAL)
        {
            postprocess_manual_check(
                    _ctx,
                    check_handle_);
        }
        else if (check_info.testsuite_handle == LibFred::TestsuiteHandle::THANK_YOU)
        {
            postprocess_thank_you_check(
                    _ctx,
                    check_handle_);
        }
    }
    catch (const LibFred::ExceptionUnknownCheckHandle&)
    {
        throw ExceptionUnknownCheckHandle();

    }
    catch (const LibFred::ExceptionUnknownCheckStatusHandle&)
    {
        throw ExceptionUnknownCheckStatusHandle();
    }
}


void resolve_check::postprocess_automatic_check(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle)
{
    LibFred::InfoContactCheckOutput check_info = LibFred::InfoContactCheck(_check_handle).exec(_ctx);

    LibFred::InfoContactOutput contact_info = LibFred::InfoContactHistoryByHistoryid(
            check_info.contact_history_id).exec(_ctx);

    const std::string& new_handle = check_info.check_state_history.rbegin()->status_handle;
    if (new_handle == LibFred::ContactCheckStatus::OK)
    {

        ContactStates::cancel_all_states(
                _ctx,
                contact_info.info_contact_data.id);

        std::set<std::string> status;
        status.insert(ContactStates::CONTACT_PASSED_MANUAL_VERIFICATION);

        std::set<unsigned long long> state_request_ids;
        state_request_ids.insert(
                LibFred::CreateObjectStateRequestId(
                        contact_info.info_contact_data.id,
                        status).exec(_ctx)
                .second);

        add_related_object_state_requests(
                _ctx,
                _check_handle,
                state_request_ids);
    }

    LibFred::PerformObjectStateRequest(contact_info.info_contact_data.id).exec(_ctx);
}


void resolve_check::postprocess_manual_check(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle)
{

    LibFred::InfoContactCheckOutput check_info = LibFred::InfoContactCheck(_check_handle).exec(_ctx);

    LibFred::InfoContactOutput contact_info = LibFred::InfoContactHistoryByHistoryid(
            check_info.contact_history_id).exec(_ctx);

    const std::string& new_handle = check_info.check_state_history.rbegin()->status_handle;

    if (new_handle != LibFred::ContactCheckStatus::OK &&
        new_handle != LibFred::ContactCheckStatus::FAIL &&
        new_handle != LibFred::ContactCheckStatus::INVALIDATED)
    {
        return;
    }

    ContactStates::cancel_all_states(
            _ctx,
            contact_info.info_contact_data.id);

    if (new_handle == LibFred::ContactCheckStatus::OK ||
        new_handle == LibFred::ContactCheckStatus::FAIL)
    {
        using namespace ContactStates;

        std::set<std::string> status;

        if (new_handle == LibFred::ContactCheckStatus::OK)
        {
            status.insert(CONTACT_PASSED_MANUAL_VERIFICATION);
        }
        else if (new_handle == LibFred::ContactCheckStatus::FAIL)
        {
            status.insert(CONTACT_FAILED_MANUAL_VERIFICATION);
        }

        std::set<unsigned long long> state_request_ids;
        state_request_ids.insert(
                LibFred::CreateObjectStateRequestId(
                        contact_info.info_contact_data.id,
                        status).exec(_ctx)
                .second);

        add_related_object_state_requests(
                _ctx,
                _check_handle,
                state_request_ids);
    }

    LibFred::PerformObjectStateRequest(contact_info.info_contact_data.id).exec(_ctx);
}


void resolve_check::postprocess_thank_you_check(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle)
{
    // it is exactly the same
    postprocess_automatic_check(
            _ctx,
            _check_handle);
}


} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
