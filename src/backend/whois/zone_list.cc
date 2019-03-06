/*
 * Copyright (C) 2015-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/whois/zone_list.hh"


namespace Fred {
namespace Backend {
namespace Whois {
    std::vector<std::string> get_managed_zone_list(
        LibFred::OperationContext& ctx)
    {
        Database::Result zone_list_res = ctx.get_conn().exec(
        "SELECT fqdn FROM zone ORDER BY fqdn");

        std::vector<std::string> ret;
        for(unsigned long long i = 0; i < zone_list_res.size(); ++i)
        {
            ret.push_back(static_cast<std::string>(zone_list_res[i]["fqdn"]));
        }
        return ret;
    }
} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred
