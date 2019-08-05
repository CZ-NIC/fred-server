/*
 * Copyright (C) 2019  CZ.NIC, z. s. p. o.
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

#include "src/bin/cli/charge_registry_access_fee.hh"

#include "src/deprecated/libfred/db_settings.hh"
#include "src/deprecated/libfred/invoicing/invoice.hh"
#include "util/util.hh"

namespace Admin {

namespace {

unsigned long long getRegistrarID(const std::string &handle) {
    Database::Connection conn = Database::Manager::acquire();
    Database::Result result = conn.exec_params(
            "SELECT id FROM registrar WHERE handle = $1::text",
            Database::query_param_list(handle));

    if (result.size() != 1) {
        throw std::runtime_error((boost::format("Registrar with handle %1% not found in database.") % handle).str());
    }

    return result[0][0];
}

} // namespace Admin::{anonymous}

void chargeRegistryAccessFee(
        bool _all_registrars,
        const std::vector<std::string>& _only_registrars,
        const std::vector<std::string>& _except_registrars,
        boost::gregorian::date _date_from,
        boost::gregorian::date _date_to)
{
    if ((_all_registrars && (!_only_registrars.empty() || !_except_registrars.empty())) ||
       (!_all_registrars && _only_registrars.empty() && _except_registrars.empty()) ||
       (!_only_registrars.empty() && !_except_registrars.empty()))
    {
        throw std::runtime_error("invalid option(s)");
    }

    std::unique_ptr<LibFred::Invoicing::Manager> invMan(
    LibFred::Invoicing::Manager::create());

    Database::Connection conn = Database::Manager::acquire();

    unsigned zone_id;
    LibFred::Invoicing::getRequestFeeParams(&zone_id);

    std::vector<Database::ID> only_registrars_ids;
    for (const auto& registrar : _only_registrars)
    {
        Database::ID reg_id = getRegistrarID(registrar);
        only_registrars_ids.push_back(reg_id);
    }
    std::string only_registrars_id_array = "{" + Util::container2comma_list(only_registrars_ids) + "}";

    std::vector<Database::ID> except_registrars_ids;
    for (const auto& registrar : _except_registrars)
    {
        Database::ID reg_id = getRegistrarID(registrar);
        except_registrars_ids.push_back(reg_id);
    }
    std::string except_registrars_id_array = "{" + Util::container2comma_list(except_registrars_ids) + "}";

    std::string condition;
    Database::query_param_list params;
    if (!_only_registrars.empty())
    {
        condition = "AND r.id = ANY ($4::bigint[]) ";
        params = Database::query_param_list(_date_from)
                                           (_date_to)
                                           (zone_id)
                                           (only_registrars_id_array);
    }
    else if (!_except_registrars.empty())
    {
        condition = "AND r.id <> ALL ($4::bigint[]) ";
        params = Database::query_param_list(_date_from)
                                           (_date_to)
                                           (zone_id)
                                           (except_registrars_id_array);
    }
    else {
        condition = "";
        params = Database::query_param_list(_date_from)
                                           (_date_to)
                                           (zone_id);
    }

    const auto result = conn.exec_params(
            "SELECT r.id "
            "FROM registrar r "
            "JOIN registrarinvoice ri "
                "ON ri.registrarid = r.id "
                "AND ( "
                    "($1::date <= ri.fromdate AND $2::date > ri.fromdate) "
                    "OR ($1::date >= ri.fromdate AND  ((ri.todate IS NULL) OR (ri.todate >= $1::date))) "
                ") "
            "WHERE r.system  = false "
                "AND ri.zone=$3::integer " +
                condition +
            "ORDER BY r.id",
            params);

    Database::Transaction tx(conn);

    for (unsigned i = 0; i < result.size(); ++i)
    {
        // let exceptions in charging terminate the whole transaction
        if (!invMan->chargeRegistryAccessFee(result[i][0], zone_id, _date_from, _date_to))
        {
            boost::format msg("Balance not sufficient for charging fee for registrar ID %1%");
            msg % result[i][0];
            LOGGER.error(msg);
            throw std::runtime_error(msg.str());
        }
    }

    tx.commit();
}

} // namespace Admin
