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

#include "src/backend/public_request/type/public_request_authinfo.hh"
#include "src/backend/public_request/type/get_iface_of.hh"
#include "libfred/public_request/public_request_status.hh"
#include "libfred/public_request/public_request_on_status_action.hh"
#include "src/backend/public_request/type/impl/implemented_by.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Type {
namespace Impl {
namespace {

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

typedef Fred::Backend::PublicRequest::Type::Impl::ImplementedBy<AuthinfoImplementation> AuthinfoPublicRequest;

extern const char authinfo_auto_rif[] = "authinfo_auto_rif";
typedef AuthinfoPublicRequest::Named<authinfo_auto_rif> AuthinfoAutoRif;

extern const char authinfo_auto_pif[] = "authinfo_auto_pif";
typedef AuthinfoPublicRequest::Named<authinfo_auto_pif> AuthinfoAuto;

extern const char authinfo_email_pif[] = "authinfo_email_pif";
typedef AuthinfoPublicRequest::Named<authinfo_email_pif> AuthinfoEmail;

extern const char authinfo_post_pif[] = "authinfo_post_pif";
typedef AuthinfoPublicRequest::Named<authinfo_post_pif> AuthinfoPost;

extern const char authinfo_government_pif[] = "authinfo_government_pif";
typedef AuthinfoPublicRequest::Named<authinfo_government_pif> AuthinfoGovernment;

} // namespace Fred::Backend::PublicRequest::Type::Impl::{anonymous}
} // namespace Fred::Backend::PublicRequest::Type::Impl

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<AuthinfoAutoRif>()
{
    static const Impl::AuthinfoAutoRif singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<AuthinfoAuto>()
{
    static const Impl::AuthinfoAuto singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<AuthinfoEmail>()
{
    static const Impl::AuthinfoEmail singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<AuthinfoPost>()
{
    static const Impl::AuthinfoPost singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<AuthinfoGovernment>()
{
    static const Impl::AuthinfoGovernment singleton;
    return singleton;
}

} // namespace Fred::Backend::PublicRequest::Type
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
