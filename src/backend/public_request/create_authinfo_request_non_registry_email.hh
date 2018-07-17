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

#ifndef CREATE_AUTHINFO_REQUEST_NON_REGISTRY_EMAIL_HH_67BB79B643874178B5A64F030C86791C
#define CREATE_AUTHINFO_REQUEST_NON_REGISTRY_EMAIL_HH_67BB79B643874178B5A64F030C86791C

#include "src/backend/public_request/confirmed_by.hh"
#include "src/backend/public_request/object_type.hh"
#include "src/libfred/opcontext.hh"
#include "src/util/optional_value.hh"

#include <string>

namespace Fred {
namespace Backend {
namespace PublicRequest {

unsigned long long create_authinfo_request_non_registry_email(
        ObjectType::Enum object_type,
        const std::string& object_handle,
        const Optional<unsigned long long>& log_request_id,
        ConfirmedBy::Enum confirmation_method,
        const std::string& specified_email);

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
