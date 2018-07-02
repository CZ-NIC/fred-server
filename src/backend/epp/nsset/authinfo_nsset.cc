/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#include "src/backend/epp/nsset/authinfo_nsset.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"
#include "src/backend/public_request/public_request.hh"
#include "src/libfred/opcontext.hh"

#include <string>

namespace Epp {
namespace Nsset {

void authinfo_nsset(
        LibFred::OperationContext& _ctx,
        const std::string& _nsset_handle,
        const SessionData& _session_data)
{
    try
    {
        const std::unique_ptr<Fred::Backend::PublicRequest::PublicRequestImpl> public_request(
                new Fred::Backend::PublicRequest::PublicRequestImpl(_server_name));

        public_request->create_authinfo_request_registry_email(
                _ctx,
                Fred::Backend::PublicRequest::PublicRequestImpl::ObjectType::nsset,
                _nsset_handle,
                _session_data.registrar_id,
                _session_data.logd_request_id);
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::ObjectNotFound&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
    }
    catch (const Fred::Backend::PublicRequest::PublicRequestImpl::NoContactEmail&)
    {
        throw EppResponseFailure(EppResultFailure(EppResultCode::command_failed));
    }
}

} // namespace Epp::Nsset
} // namespace Epp
