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

#ifndef PUBLIC_REQUEST_PERSONALINFO_HH_A366D7960D55468A86355804EEA01FC2
#define PUBLIC_REQUEST_PERSONALINFO_HH_A366D7960D55468A86355804EEA01FC2

#include "src/libfred/public_request/public_request_type_iface.hh"
#include "src/libfred/public_request/public_request_status.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"
#include "src/backend/public_request/public_request.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Type {
namespace Blockunblock {

const LibFred::PublicRequestTypeIface& get_block_changes_email_iface();
const LibFred::PublicRequestTypeIface& get_block_changes_post_iface();
const LibFred::PublicRequestTypeIface& get_block_transfer_email_iface();
const LibFred::PublicRequestTypeIface& get_block_transfer_post_iface();
const LibFred::PublicRequestTypeIface& get_unblock_changes_email_iface();
const LibFred::PublicRequestTypeIface& get_unblock_changes_post_iface();
const LibFred::PublicRequestTypeIface& get_unblock_transfer_email_iface();
const LibFred::PublicRequestTypeIface& get_unblock_transfer_post_iface();

const LibFred::PublicRequestTypeIface& get_block_transfer_iface(PublicRequestImpl::ConfirmedBy::Enum confirmation_method);
const LibFred::PublicRequestTypeIface& get_block_change_iface(PublicRequestImpl::ConfirmedBy::Enum confirmation_method);
const LibFred::PublicRequestTypeIface& get_unblock_transfer_iface(PublicRequestImpl::ConfirmedBy::Enum confirmation_method);
const LibFred::PublicRequestTypeIface& get_unblock_changes_iface(PublicRequestImpl::ConfirmedBy::Enum confirmation_method);

} // namespace Fred::Backend::PublicRequest::Type::Blockunblock::{anonymous}
} // namespace Fred::Backend::PublicRequest::Type::{anonymous}
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
