/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/public_request/type/public_request_personal_info.hh"
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

struct PersonalInfoImplementation
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

typedef Fred::Backend::PublicRequest::Type::Impl::ImplementedBy<PersonalInfoImplementation> PersonalInfoPublicRequest;

extern const char personal_info_auto_pif[] = "personalinfo_auto_pif";
typedef PersonalInfoPublicRequest::Named<personal_info_auto_pif> PersonalInfoAutoType;

extern const char personal_info_email_pif[] = "personalinfo_email_pif";
typedef PersonalInfoPublicRequest::Named<personal_info_email_pif> PersonalInfoEmailType;

extern const char personal_info_post_pif[] = "personalinfo_post_pif";
typedef PersonalInfoPublicRequest::Named<personal_info_post_pif> PersonalInfoPostType;

extern const char personal_info_government_pif[] = "personalinfo_government_pif";
typedef PersonalInfoPublicRequest::Named<personal_info_government_pif> PersonalInfoGovernmentType;

} // namespace Fred::Backend::PublicRequest::Type::Impl::{anonymous}
} // namespace Fred::Backend::PublicRequest::Type::Impl

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<PersonalInfoAuto>()
{
    static const Impl::PersonalInfoAutoType singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<PersonalInfoEmail>()
{
    static const Impl::PersonalInfoEmailType singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<PersonalInfoPost>()
{
    static const Impl::PersonalInfoPostType singleton;
    return singleton;
}

template<>
const LibFred::PublicRequestTypeIface& get_iface_of<PersonalInfoGovernment>()
{
    static const Impl::PersonalInfoGovernmentType singleton;
    return singleton;
}

} // namespace Fred::Backend::PublicRequest::Type
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
