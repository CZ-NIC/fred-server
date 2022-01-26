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

#ifndef PUBLIC_REQUEST_AUTHINFO_IMPL_HH_0C27FF53A0AD41F195A8864D0B21C4B4
#define PUBLIC_REQUEST_AUTHINFO_IMPL_HH_0C27FF53A0AD41F195A8864D0B21C4B4

#include "src/deprecated/libfred/public_request/public_request.hh"


namespace LibFred {
namespace PublicRequest {

static const Type PRT_AUTHINFO_AUTO_RIF = "authinfo_auto_rif";
static const Type PRT_AUTHINFO_AUTO_PIF = "authinfo_auto_pif";
static const Type PRT_AUTHINFO_EMAIL_PIF = "authinfo_email_pif";
static const Type PRT_AUTHINFO_POST_PIF = "authinfo_post_pif";
static const Type PRT_AUTHINFO_GOVERNMENT_PIF = "authinfo_government_pif";

Factory& add_authinfo_producers(Factory& factory);

} // namespace LibFred::PublicRequest
} // namespace LibFred

#endif//PUBLIC_REQUEST_AUTHINFO_IMPL_HH_0C27FF53A0AD41F195A8864D0B21C4B4
