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

#ifndef PUBLIC_REQUESTS_HH_E22CB50FF8D84815ACE87616441DA7BA
#define PUBLIC_REQUESTS_HH_E22CB50FF8D84815ACE87616441DA7BA

#include "src/backend/public_request/object_type.hh"
#include "src/libfred/opcontext.hh"
#include "src/util/optional_value.hh"

#include <string>

namespace Fred {
namespace Backend {
namespace PublicRequest {

unsigned long long create_authinfo_request_registry_email_rif(
        LibFred::OperationContext& _ctx,
        ObjectType::Enum _object_type,
        const std::string& _object_handle,
        unsigned long long _registrar_id,
        const Optional<unsigned long long>& _log_request_id);

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
