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

#ifndef _ENUMPARAMCLIENT_H_
#define _ENUMPARAMCLIENT_H_

#define ENUMPARAM_ENUM_PARAMETER_CHANGE         "enum_parameter_change"
#define ENUMPARAM_ENUM_PARAMETER_CHANGE_DESC    "change value of enum_parameter by name"
#define ENUMPARAM_NAME                          "parameter_name"
#define ENUMPARAM_NAME_DESC                     "enum parameter name"
#define ENUMPARAM_VALUE                         "parameter_value"
#define ENUMPARAM_VALUE_DESC                    "enum parameter value"



#include <boost/program_options.hpp>
#include <iostream>

#include "old_utils/dbsql.h"
#include "register/register.h"

#include "corba/admin/admin_impl.h"
#include "baseclient.h"


namespace Admin
{

class EnumParamClient : public BaseClient {
private:
    CORBA::Long m_clientId;
    DB m_db;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    static const struct options m_opts[];
public:
    EnumParamClient()
    { }
    EnumParamClient(
            const std::string &connstring,
            const std::string &nsAddr,
            const Config::Conf &conf):
        BaseClient(connstring, nsAddr),
        m_conf(conf)
    {
        m_db.OpenDatabase(connstring.c_str());
    }
    ~EnumParamClient()
    { }

    static const struct options *getOpts();
    static int getOptsCount();
    void runMethod();

    void show_opts();
    void enum_parameter_change();

    void enum_parameter_change_help();

}; // class EnumParamClient


}; // namespace Admin

#endif // _ENUMPARAMCLIENT_H_
