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

#ifndef PUBLIC_REQUEST_AUTHINFO_HH_6A9E148ADE9D445C891AC330DC1E7634
#define PUBLIC_REQUEST_AUTHINFO_HH_6A9E148ADE9D445C891AC330DC1E7634

#include "src/libfred/public_request/public_request_type_iface.hh"
#include "src/libfred/object/object_states_info.hh"
#include "src/backend/public_request/public_request.hh"
#include "src/backend/public_request/type/get_iface_of.hh"

#include <string>
#include <memory>

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Type {

struct AuthinfoAuto;
struct AuthinfoEmail;
struct AuthinfoPost;

unsigned long long send_authinfo(
        unsigned long long public_request_id,
        const std::string& handle,
        PublicRequestImpl::ObjectType::Enum object_type,
        std::shared_ptr<LibFred::Mailer::Manager> manager);

void check_authinfo_request_permission(const LibFred::ObjectStatesInfo& states);

} // namespace Fred::Backend::PublicRequest::Type
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
