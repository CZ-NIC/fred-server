/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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

#include <string>
#include <vector>

namespace Admin
{

void
EnumParamClient::runMethod()
{
    if (enum_parameter_change_//m_conf.hasOpt(ENUMPARAM_ENUM_PARAMETER_CHANGE)
            ) {
        enum_parameter_change();
    }
}


void
EnumParamClient::enum_parameter_change()
{

    std::string name = enum_parameter_change_params.parameter_name;//m_conf.get<std::string>(ENUMPARAM_NAME);
    std::string value = enum_parameter_change_params.parameter_value;//m_conf.get<std::string>(ENUMPARAM_VALUE);

    try {
        Database::Connection conn = Database::Manager::acquire();

        std::stringstream squery;
        squery << "select id from enum_parameters where name = "
               << Database::Value(name);
        Database::Result exist = conn.exec(squery.str());
        if (exist.size() == 0) {
            throw std::runtime_error(str(boost::format(
                            "parameter '%1%' not found")
                            % name));
        }

        std::stringstream query;
        query << "update enum_parameters set val = '" << conn.escape(value) << "'"
              << " where name = '" << conn.escape(name) << "'";
        conn.exec(query.str());
    }
    catch (std::exception &ex) {
        throw std::runtime_error(str(boost::format(
                        "enum parameter change: %1%")
                        % ex.what()));
    }
    catch (...) {
        throw std::runtime_error("enum parameter change: unknown error");
    }

}

}; // namespace Admin
