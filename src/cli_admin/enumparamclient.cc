/*
 *  Copyright (C) 2008, 2009  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "simple.h"
#include "commonclient.h"
#include "enumparamclient.h"
#include "register/db_settings.h"


#include <string>
#include <vector>

namespace Admin
{

const struct options *
EnumParamClient::getOpts()
{
    return m_opts;
}

void
EnumParamClient::runMethod()
{
    if (m_conf.hasOpt(ENUMPARAM_ENUM_PARAMETER_CHANGE)) {
        enum_parameter_change();
    }
}

void
EnumParamClient::show_opts()
{
    callHelp(m_conf, no_help);
    print_options("EnumParam", getOpts(), getOptsCount());
}

void
EnumParamClient::enum_parameter_change()
{
    callHelp(m_conf, enum_parameter_change_help);

    try
    {
        std::string name = m_conf.get<std::string>(ENUMPARAM_NAME);
        std::string value = m_conf.get<std::string>(ENUMPARAM_VALUE);

        Database::Connection conn = Database::Manager::acquire();
        std::stringstream query;
        query << "update enum_parameters set val = '" << conn.escape(value) << "'"
              << " where name = '" << conn.escape(name) << "'";
        Database::Transaction tx(conn);
        conn.exec(query.str());
        tx.commit();
    }
    catch (...)
    {
        std::cerr << "An error has occured" << std::endl;
    }
}
void
EnumParamClient::enum_parameter_change_help()
{
    std::cout <<
        "** Change enum parameter by name **\n\n"
        "  $ " << g_prog_name << " --" << ENUMPARAM_ENUM_PARAMETER_CHANGE << " \\\n"
        "    --" << ENUMPARAM_NAME << "=<enum_parameter_name> \\\n"
        "    --" << ENUMPARAM_VALUE << "=<enum_parameter_value> \\\n"
        << std::endl;
}

#define ADDOPT(name, type, callable, visible) \
    {CLIENT_ENUMPARAM, name, name##_DESC, type, callable, visible}

const struct options
EnumParamClient::m_opts[] = {
    ADDOPT(ENUMPARAM_ENUM_PARAMETER_CHANGE, TYPE_NOTYPE, true, true),
    ADDOPT(ENUMPARAM_NAME, TYPE_STRING, false, false),
    ADDOPT(ENUMPARAM_VALUE, TYPE_STRING, false, false)
    };

#undef ADDOPT

int
EnumParamClient::getOptsCount()
{
    return sizeof(m_opts) / sizeof(options);
}



}; // namespace Admin
