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

#include "src/backend/public_request/type/personalinfo/public_request_personalinfo.hh"
#include "src/backend/public_request/type/impl/public_request_impl.hh"
#include "src/backend/public_request/public_request.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Type {

namespace {

struct PersonalinfoImplementation
{
    template <typename T>
    LibFred::PublicRequestTypeIface::PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        return LibFred::PublicRequestTypeIface::PublicRequestTypes();
    }
    template <typename T>
    LibFred::PublicRequestTypeIface::PublicRequestTypes get_public_request_types_to_cancel_on_update(
            LibFred::PublicRequest::Status::Enum,
            LibFred::PublicRequest::Status::Enum) const
    {
        return LibFred::PublicRequestTypeIface::PublicRequestTypes();
    }

    template <typename T>
    LibFred::PublicRequest::OnStatusAction::Enum get_on_status_action(LibFred::PublicRequest::Status::Enum _status) const
    {
        if (_status == LibFred::PublicRequest::Status::resolved)
        {
            return LibFred::PublicRequest::OnStatusAction::scheduled;
        }
        return LibFred::PublicRequest::OnStatusAction::processed;
    }
};

typedef ImplementedBy<PersonalinfoImplementation> PersonalinfoPublicRequest;

extern const char personalinfo_auto_pif[] = "personalinfo_auto_pif";
typedef PersonalinfoPublicRequest::Named<personalinfo_auto_pif> PersonalinfoAuto;

extern const char personalinfo_email_pif[] = "personalinfo_email_pif";
typedef PersonalinfoPublicRequest::Named<personalinfo_email_pif> PersonalinfoEmail;

extern const char personalinfo_post_pif[] = "personalinfo_post_pif";
typedef PersonalinfoPublicRequest::Named<personalinfo_post_pif> PersonalinfoPost;

} // namespace Fred::Backend::PublicRequest::Type::{anonymous}
} // namespace Fred::Backend::PublicRequest::Type

const LibFred::PublicRequestTypeIface& get_personal_info_auto_iface()
{
    static const Type::PersonalinfoAuto singleton;
    return singleton;
}

const LibFred::PublicRequestTypeIface& get_personal_info_email_iface()
{
    static const Type::PersonalinfoEmail singleton;
    return singleton;
}

const LibFred::PublicRequestTypeIface& get_personal_info_post_iface()
{
    static const Type::PersonalinfoPost singleton;
    return singleton;
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
