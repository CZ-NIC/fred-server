/*
 *  Copyright (C) 2008  CZ.NIC, z.s.p.o.
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

#ifndef _REGISTRARCLIENT_H_
#define _REGISTRARCLIENT_H_

#define REGISTRAR_LIST_NAME                 "registrar-list"
#define REGISTRAR_LIST_NAME_DESC            "list all registrars (via filters)"
#define REGISTRAR_ZONE_ADD_NAME             "add-zone"
#define REGISTRAR_REGISTRAR_ADD_NAME        "add-registrar"
#define REGISTRAR_REGISTRAR_ADD_ZONE_NAME   "add-registrar-to-zone"

#define REGISTRAR_ZONE_ADD_HELP_NAME             "add-zone-help"
#define REGISTRAR_REGISTRAR_ADD_HELP_NAME        "add-registrar-help"
#define REGISTRAR_REGISTRAR_ADD_ZONE_HELP_NAME   "add-registrar-to-zone-help"


#include <boost/program_options.hpp>
#include <iostream>

#include "old_utils/dbsql.h"
#include "register/register.h"

#include "corba/admin/admin_impl.h"

namespace Admin {

class RegistrarClient {
private:
    std::string m_connstring;
    std::string m_nsAddr;
    CORBA::Long m_clientId;
    DB m_db;
    Database::Manager *m_dbman;
    boost::program_options::variables_map m_varMap;
    ccReg::EPP_var m_epp;

    boost::program_options::options_description *m_options;
    boost::program_options::options_description *m_optionsInvis;
public:
    RegistrarClient();
    RegistrarClient(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);
    ~RegistrarClient();
    void init(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;

    void list();
    int zone_add();
    int registrar_add();
    int registrar_add_zone();

    void zone_add_help();
    void registrar_add_help();
    void registrar_add_zone_help();
};

} // namespace Admin;

#endif // _REGISTRARCLIENT_H_
