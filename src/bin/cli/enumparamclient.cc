/*
 * Copyright (C) 2010-2020  CZ.NIC, z. s. p. o.
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

#include "src/bin/cli/commonclient.hh"
#include "src/bin/cli/enumparamclient.hh"

#include "libfred/db_settings.hh"

#include <set>
#include <string>
#include <stdexcept>

namespace Admin {

void EnumParamClient::runMethod()
{
    if (enum_parameter_change_)//m_conf.hasOpt(ENUMPARAM_ENUM_PARAMETER_CHANGE)
    {
        enum_parameter_change();
    }
}


void EnumParamClient::enum_parameter_change()
{
    try
    {
        const std::string name = enum_parameter_change_params.parameter_name;//m_conf.get<std::string>(ENUMPARAM_NAME);
        static const std::set<std::string> domain_lifecycle_parameters =
            {
                "expiration_notify_period",
                "outzone_unguarded_email_warning_period",
                "expiration_dns_protection_period",
                "expiration_letter_warning_period",
                "expiration_registration_protection_period",
                "validation_notify1_period",
                "validation_notify2_period"
            };
        if (0 < domain_lifecycle_parameters.count(name))
        {
            throw std::runtime_error("parameter '" + name + "' moved into domain_lifecycle_parameters");
        }
        const std::string value = enum_parameter_change_params.parameter_value;//m_conf.get<std::string>(ENUMPARAM_VALUE);
        Database::Connection conn = Database::Manager::acquire();

        const std::string query =
                "UPDATE enum_parameters "
                "SET val = '" + conn.escape(value) + "' "
                "WHERE name = '" + conn.escape(name) + "' "
                "RETURNING 0";
        const auto dbres = conn.exec(query);
        if (dbres.size() == 0)
        {
            throw std::runtime_error(str(boost::format("parameter '%1%' not found") % name));
        }
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(str(boost::format("enum parameter change: %1%") % e.what()));
    }
    catch (...)
    {
        throw std::runtime_error("enum parameter change: unknown error");
    }
}

} // namespace Admin
