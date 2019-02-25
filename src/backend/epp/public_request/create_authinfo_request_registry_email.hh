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
#ifndef CREATE_AUTHINFO_REQUEST_REGISTRY_EMAIL_HH_E22CB50FF8D84815ACE87616441DA7BA
#define CREATE_AUTHINFO_REQUEST_REGISTRY_EMAIL_HH_E22CB50FF8D84815ACE87616441DA7BA

#include "src/backend/public_request/object_type.hh"
#include "libfred/opcontext.hh"
#include "util/optional_value.hh"

#include <string>

namespace Epp {
namespace PublicRequest {

unsigned long long create_authinfo_request_registry_email(
        LibFred::OperationContext& _ctx,
        Fred::Backend::PublicRequest::ObjectType _object_type,
        const std::string& _object_handle,
        unsigned long long _registrar_id,
        const Optional<unsigned long long>& _log_request_id);

} // namespace Epp::PublicRequest
} // namespace Epp

#endif
