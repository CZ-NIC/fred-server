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
#ifndef PUBLIC_REQUEST_PERSONAL_INFO_IMPL_HH_80066B98FFF38FCAF44A43B14027DC88
#define PUBLIC_REQUEST_PERSONAL_INFO_IMPL_HH_80066B98FFF38FCAF44A43B14027DC88

#include "src/deprecated/libfred/public_request/public_request.hh"
#include "util/factory.hh"


namespace LibFred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DECL(personal_info)


static const Type PRT_PERSONALINFO_AUTO_PIF = "personalinfo_auto_pif";
static const Type PRT_PERSONALINFO_EMAIL_PIF = "personalinfo_email_pif";
static const Type PRT_PERSONALINFO_POST_PIF = "personalinfo_post_pif";
static const Type PRT_PERSONALINFO_GOVERNMENT_PIF = "personalinfo_government_pif";

} // namespace LibFred::PublicRequest
} // namespace LibFred

#endif
