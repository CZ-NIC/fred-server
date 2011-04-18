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

#include "fredlib/registry.h"

#include "corba/admin/admin_impl.h"
#include "baseclient.h"

#include "enumparam_params.h"


namespace Admin
{

class EnumParamClient : public BaseClient {
private:
    ccReg::EPP_var m_epp;

    bool enum_parameter_change_;
    EnumParameterChangeArgs enum_parameter_change_params;

    static const struct options m_opts[];
public:
    EnumParamClient()
    : enum_parameter_change_(false)
    { }
    EnumParamClient(
            const std::string &connstring
            , const std::string &nsAddr
            , bool _enum_parameter_change
            , const EnumParameterChangeArgs& _enum_parameter_change_params
            )
        : BaseClient(connstring, nsAddr)
        , enum_parameter_change_(_enum_parameter_change)
        , enum_parameter_change_params(_enum_parameter_change_params)
        { }
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
