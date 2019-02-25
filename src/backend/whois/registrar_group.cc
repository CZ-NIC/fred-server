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
#include "src/backend/whois/registrar_group.hh"


namespace Fred {
namespace Backend {
namespace Whois {

std::map<std::string, std::vector<std::string> > get_registrar_groups(
        LibFred::OperationContext& ctx)
{
    Database::Result registrar_groups_res = ctx.get_conn().exec(
            // clang-format off
        "SELECT rg.short_name AS registrar_group, "
                "r.handle AS registrar_handle "
            "FROM registrar_group rg "
                "JOIN registrar_group_map rgm ON rg.id = rgm.registrar_group_id "
                    "AND rg.cancelled IS NULL "
                "JOIN registrar r ON r.id = rgm.registrar_id "
                    "AND rgm.member_from <= CURRENT_DATE "
                    "AND (rgm.member_until IS NULL "
                        "OR (rgm.member_until >= CURRENT_DATE "
                        "AND rgm.member_from <> rgm.member_until))");
    // clang-format on

    std::map<std::string, std::vector<std::string> > ret;
    for (unsigned long long i = 0; i < registrar_groups_res.size(); ++i)
    {
        ret[static_cast<std::string>(registrar_groups_res[i]["registrar_group"])].push_back(
                static_cast<std::string>(registrar_groups_res[i]["registrar_handle"]));
    }
    return ret;
}

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred
