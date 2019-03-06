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
#ifndef PUBLIC_REQUEST_AUTHINFO_HH_6A9E148ADE9D445C891AC330DC1E7634
#define PUBLIC_REQUEST_AUTHINFO_HH_6A9E148ADE9D445C891AC330DC1E7634

#include "libfred/public_request/public_request_type_iface.hh"
#include "libfred/object/object_states_info.hh"
#include "src/backend/public_request/object_type.hh"

#include <string>
#include <memory>

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Type {

struct AuthinfoAutoRif;
struct AuthinfoAuto;
struct AuthinfoEmail;
struct AuthinfoPost;
struct AuthinfoGovernment;

} // namespace Fred::Backend::PublicRequest::Type
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
