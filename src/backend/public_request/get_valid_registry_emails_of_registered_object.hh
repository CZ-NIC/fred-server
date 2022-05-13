/*
 * Copyright (C) 2018-2022  CZ.NIC, z. s. p. o.
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
#ifndef GET_VALID_REGISTRY_EMAILS_OF_REGISTERED_OBJECT_HH_368F424059874406A9FA167E52DC3804
#define GET_VALID_REGISTRY_EMAILS_OF_REGISTERED_OBJECT_HH_368F424059874406A9FA167E52DC3804

#include "src/backend/public_request/object_type.hh"
#include "src/backend/public_request/util/send_joined_address_email.hh"

#include "libfred/opcontext.hh"

#include <boost/uuid/uuid.hpp>

#include <set>

namespace Fred {
namespace Backend {
namespace PublicRequest {

std::set<Util::EmailData::Recipient> get_valid_registry_emails_of_registered_object(
        LibFred::OperationContext& _ctx,
        ObjectType _object_type,
        unsigned long long _object_id);

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
