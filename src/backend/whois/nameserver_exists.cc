/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/whois/nameserver_exists.hh"


namespace Fred {
namespace Backend {
namespace Whois {

bool nameserver_exists(const std::string& ns_fqdn, LibFred::OperationContext& ctx)
{

    return ctx.get_conn().exec_params(
                                 "SELECT 123 FROM host WHERE fqdn = lower($1::varchar) LIMIT 1",
                                 Database::query_param_list(ns_fqdn))
                   .size() > 0;
}

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred
