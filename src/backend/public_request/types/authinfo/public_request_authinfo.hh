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

#ifndef PUBLIC_REQUEST_AUTHINFO_HH_8B3EE0B3BE664879ABB060A09093D67C
#define PUBLIC_REQUEST_AUTHINFO_HH_8B3EE0B3BE664879ABB060A09093D67C

#include "src/libfred/public_request/public_request_type_iface.hh"
#include "src/libfred/public_request/public_request_status.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"
#include "src/libfred/object/object_states_info.hh"
#include "src/backend/public_request/public_request.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Type {

struct AuthinfoImplementation
{
    template <typename T>
    LibFred::PublicRequestTypeIface::PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        LibFred::PublicRequestTypeIface::PublicRequestTypes res;
        res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new T));
        return res;
    }
    template <typename T>
    LibFred::PublicRequestTypeIface::PublicRequestTypes get_public_request_types_to_cancel_on_update(
            LibFred::PublicRequest::Status::Enum,
            LibFred::PublicRequest::Status::Enum) const
    {
        return LibFred::PublicRequestTypeIface::PublicRequestTypes();
    };
    template <typename T>
    LibFred::PublicRequest::OnStatusAction::Enum get_on_status_action(LibFred::PublicRequest::Status::Enum _status) const
    {
        return LibFred::PublicRequest::OnStatusAction::processed;
    };
};

} // Fred::Backend::PublicRequest::Type

unsigned long long send_authinfo(
        unsigned long long public_request_id,
        const std::string& handle,
        PublicRequestImpl::ObjectType::Enum object_type,
        std::shared_ptr<LibFred::Mailer::Manager> manager);

void check_authinfo_request_permission(const LibFred::ObjectStatesInfo& states);

const LibFred::PublicRequestTypeIface& get_auth_info_auto_iface();
const LibFred::PublicRequestTypeIface& get_auth_info_email_iface();
const LibFred::PublicRequestTypeIface& get_auth_info_post_iface();

} // Fred::Backend::PublicRequest
} // Fred::Backend
} // Fred

#endif
