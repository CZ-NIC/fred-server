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

#ifndef CREATE_PERSONAL_INFO_REQUEST_NON_REGISTRY_EMAIL_HH_CB2032238CDC4AD795D535C947A65199
#define CREATE_PERSONAL_INFO_REQUEST_NON_REGISTRY_EMAIL_HH_CB2032238CDC4AD795D535C947A65199

#include "src/backend/public_request/confirmed_by.hh"
#include "src/backend/public_request/object_type.hh"
#include "src/libfred/opcontext.hh"
#include "src/util/optional_value.hh"

#include <string>

namespace Fred {
namespace Backend {
namespace PublicRequest {

unsigned long long create_personal_info_request_non_registry_email(
        const std::string& contact_handle,
        const Optional<unsigned long long>& log_request_id,
        ConfirmedBy confirmation_method,
        const std::string& specified_email);

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
