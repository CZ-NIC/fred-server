/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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

#ifndef GET_PUBLIC_REQUEST_UUID_HH_89C0714FE8E74B319CF72B5234B72D43
#define GET_PUBLIC_REQUEST_UUID_HH_89C0714FE8E74B319CF72B5234B72D43

#include "libfred/object_state/typedefs.hh"
#include "libfred/opcontext.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Util {

boost::uuids::uuid get_public_request_uuid(
        LibFred::OperationContext& _ctx,
        const LibFred::PublicRequestId& _public_request_id);

} // namespace Fred::Backend::PublicRequest::Util
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
