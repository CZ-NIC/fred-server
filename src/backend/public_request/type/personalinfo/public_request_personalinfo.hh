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

#ifndef PUBLIC_REQUEST_PERSONALINFO_HH_D1B2A453ABD94CD1A241FDEED46A3026
#define PUBLIC_REQUEST_PERSONALINFO_HH_D1B2A453ABD94CD1A241FDEED46A3026

#include "src/libfred/public_request/public_request_type_iface.hh"
#include "src/libfred/public_request/public_request_status.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {

const LibFred::PublicRequestTypeIface& get_personal_info_auto_iface();
const LibFred::PublicRequestTypeIface& get_personal_info_email_iface();
const LibFred::PublicRequestTypeIface& get_personal_info_post_iface();

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
