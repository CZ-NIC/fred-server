/*
 * Copyright (C) 2011-2022  CZ.NIC, z. s. p. o.
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

#ifndef PUBLIC_REQUEST_BLOCK_IMPL_HH_6737A5987F404D159FEE30FE07322386
#define PUBLIC_REQUEST_BLOCK_IMPL_HH_6737A5987F404D159FEE30FE07322386

#include "src/deprecated/libfred/public_request/public_request.hh"


namespace LibFred {
namespace PublicRequest {

static const Type PRT_BLOCK_TRANSFER_EMAIL_PIF = "block_transfer_email_pif";
static const Type PRT_BLOCK_CHANGES_EMAIL_PIF = "block_changes_email_pif";
static const Type PRT_UNBLOCK_TRANSFER_EMAIL_PIF = "unblock_transfer_email_pif";
static const Type PRT_UNBLOCK_CHANGES_EMAIL_PIF = "unblock_changes_email_pif";
static const Type PRT_BLOCK_TRANSFER_POST_PIF = "block_transfer_post_pif";
static const Type PRT_BLOCK_CHANGES_POST_PIF = "block_changes_post_pif";
static const Type PRT_UNBLOCK_TRANSFER_POST_PIF = "unblock_transfer_post_pif";
static const Type PRT_UNBLOCK_CHANGES_POST_PIF = "unblock_changes_post_pif";
static const Type PRT_BLOCK_TRANSFER_GOVERNMENT_PIF = "block_transfer_government_pif";
static const Type PRT_BLOCK_CHANGES_GOVERNMENT_PIF = "block_changes_government_pif";
static const Type PRT_UNBLOCK_TRANSFER_GOVERNMENT_PIF = "unblock_transfer_government_pif";
static const Type PRT_UNBLOCK_CHANGES_GOVERNMENT_PIF = "unblock_changes_government_pif";

Factory& add_block_producers(Factory& factory);

} // namespace LibFred::PublicRequest
} // namespace LibFred

#endif//PUBLIC_REQUEST_BLOCK_IMPL_HH_6737A5987F404D159FEE30FE07322386
