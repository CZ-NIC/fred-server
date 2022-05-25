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

#include "src/backend/public_request/util/get_public_request_uuid.hh"
#include "src/backend/public_request/exceptions.hh"

#include <boost/uuid/uuid.hpp>

#include <string>

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Util {

boost::uuids::uuid get_public_request_uuid(
        LibFred::OperationContext& _ctx,
        const LibFred::PublicRequestId& _public_request_id)
{
    const std::string sql_query =
    // clang-format off
            "SELECT pr.uuid "
              "FROM public_request pr "
            "WHERE pr.id = $1::BIGINT";
    // clang-format on
    const Database::Result db_result = _ctx.get_conn().exec_params(
            sql_query,
            Database::query_param_list(_public_request_id));
    if (db_result.size() < 1)
    {
        throw ObjectNotFound();
    }
    if (db_result.size() > 1)
    {
        throw std::runtime_error("too many objects for given id");
    }
    auto public_request_uuid = boost::uuids::string_generator{}(static_cast<std::string>(db_result[0]["uuid"]));
    return public_request_uuid;
}


} // namespace Fred::Backend::PublicRequest::Util
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
