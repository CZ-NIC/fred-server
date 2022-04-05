/*
 * Copyright (C) 2012-2022  CZ.NIC, z. s. p. o.
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

#ifndef PUBLIC_REQUEST_CONTACT_VERIFICATION_IMPL_HH_91F6DA6A61164878BE2AC425B01B71AA
#define PUBLIC_REQUEST_CONTACT_VERIFICATION_IMPL_HH_91F6DA6A61164878BE2AC425B01B71AA

#include "src/deprecated/libfred/public_request/public_request.hh"

namespace Fred {
namespace Backend {
namespace ContactVerification {
namespace PublicRequest {

static const LibFred::PublicRequest::Type PRT_CONTACT_CONDITIONAL_IDENTIFICATION = "contact_conditional_identification";
static const LibFred::PublicRequest::Type PRT_CONTACT_IDENTIFICATION = "contact_identification";

LibFred::PublicRequest::Factory& add_producers(LibFred::PublicRequest::Factory& factory);

} // namespace Fred::Backend::ContactVerification::PublicRequest
} // namespace Fred::Backend::ContactVerification
} // namespace Fred::Backend
} // namespace Fred

#endif//CONTACT_VERIFICATION_PUBLIC_REQUEST_IMPL_H_F64C96477171CEB486A675DFFEF62965

