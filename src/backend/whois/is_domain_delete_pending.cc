/*
 * Copyright (C) 2014-2021  CZ.NIC, z. s. p. o.
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

#include "src/backend/whois/is_domain_delete_pending.hh"

namespace Fred {
namespace Backend {
namespace Whois {

bool is_domain_delete_pending(const std::string& _fqdn, LibFred::OperationContext& _ctx, const std::string& _timezone)
{
    const Database::Result result = _ctx.get_conn().exec_params(
            // clang-format off
            "WITH erdate_interval AS ("
                // conversions are necessary because we are interested in start/end of day in local time zone
                "SELECT date_trunc('day', NOW() AT TIME ZONE 'UTC' AT TIME ZONE $2::text) AS from_, "
                       "date_trunc('day', NOW() AT TIME ZONE 'UTC' AT TIME ZONE $2::text + '1DAY'::INTERVAL) AS to_) "
            "SELECT oreg.id "
            "FROM object_registry AS oreg "
            "JOIN object_state AS os ON os.object_id = oreg.id "
            "JOIN enum_object_states eos ON eos.name = 'deleteCandidate' AND "
                                           "eos.id = os.state_id, "
                 "erdate_interval "
            "WHERE oreg.name = LOWER($1::text) AND "
                  "oreg.type = get_object_type_id('domain') AND "
                  "(oreg.erdate IS NULL OR "
                   "(erdate_interval.from_ <= oreg.erdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE $2::text AND "
                                             "oreg.erdate::timestamp AT TIME ZONE 'UTC' AT TIME ZONE $2::text < erdate_interval.to_)) AND "
                  "os.valid_from <= NOW() AND "
                                  "(NOW() < os.valid_to OR os.valid_to IS NULL)",
            // clang-format on
            Database::query_param_list
                (_fqdn)
                (_timezone));

    if (result.size() == 0)
    {
        return false;
    }

    const auto pending_domain_id = static_cast<unsigned long long>(result[0][0]);
    LOGGER.debug(boost::format("delete pending check for fqdn %1% selected id=%2%") % _fqdn
                                                                                    % pending_domain_id);
    const Database::Result check = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT id "
            "FROM object_registry "
            "WHERE type = get_object_type_id('domain') AND "
                  "name = LOWER($1::text) AND "
                  "id != $2::bigint AND "
                  "(SELECT crdate FROM object_registry WHERE id = $2::bigint) < crdate "
            "ORDER BY crdate DESC "
            "LIMIT 1",
            // clang-format on
            Database::query_param_list
                (_fqdn)
                (pending_domain_id));

    if (check.size() == 0)
    {
        return true;
    }
    LOGGER.debug(boost::format("delete pending check found newer domain %1% with id=%2%") % _fqdn
                                                                                          % static_cast<unsigned long long>(check[0][0]));
    return false;
}

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred
