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

#ifndef PUBLIC_REQUEST_BLOCKUNBLOCK_HH_53EF83ABD342468AB0B386243FC06F33
#define PUBLIC_REQUEST_BLOCKUNBLOCK_HH_53EF83ABD342468AB0B386243FC06F33

#include "src/libfred/public_request/public_request_type_iface.hh"
#include "src/backend/public_request/public_request.hh"
#include "src/backend/public_request/type/get_iface_of.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Type {

struct BlockChangesEmail;
struct BlockChangesPost;
struct BlockTransferEmail;
struct BlockTransferPost;

struct UnblockChangesEmail;
struct UnblockChangesPost;
struct UnblockTransferEmail;
struct UnblockTransferPost;

const LibFred::PublicRequestTypeIface& get_block_transfer_iface(PublicRequestImpl::ConfirmedBy::Enum confirmation_method);
const LibFred::PublicRequestTypeIface& get_block_change_iface(PublicRequestImpl::ConfirmedBy::Enum confirmation_method);
const LibFred::PublicRequestTypeIface& get_unblock_transfer_iface(PublicRequestImpl::ConfirmedBy::Enum confirmation_method);
const LibFred::PublicRequestTypeIface& get_unblock_changes_iface(PublicRequestImpl::ConfirmedBy::Enum confirmation_method);

} // namespace Fred::Backend::PublicRequest::Type
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
