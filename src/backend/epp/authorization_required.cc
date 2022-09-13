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

#include "src/backend/epp/authorization_required.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/epp_result_failure.hh"

#include "libfred/object/check_authinfo.hh"
#include "util/log/logger.hh"

namespace Epp {

void authorization_required(
        LibFred::OperationContext& ctx,
        LibFred::Object::ObjectId object_id,
        const Password& password)
{
    const auto is_authorized = 0 < LibFred::Object::CheckAuthinfo{object_id}.exec(
            ctx,
            *password,
            LibFred::Object::CheckAuthinfo::increment_usage);
    if (!is_authorized)
    {
        LOGGER.info("password does not match for object " + std::to_string(*object_id));
        throw EppResponseFailure{EppResultFailure{EppResultCode::invalid_authorization_information}};
    }
}

} // namespace Epp
