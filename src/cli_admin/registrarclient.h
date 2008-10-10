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

#define REGISTRAR_SHOW_OPTS_NAME            "registrar_show_opts"
#define REGISTRAR_SHOW_OPTS_NAME_DESC       "show all registrar command line options"
#define REGISTRAR_LIST_NAME                 "registrar_list"
#define REGISTRAR_LIST_NAME_DESC            "list all registrars (via filters)"
#define REGISTRAR_ZONE_ADD_NAME             "zone_add"
#define REGISTRAR_ZONE_ADD_NAME_DESC        "add new zone"
#define REGISTRAR_REGISTRAR_ADD_NAME        "registrar_add"
#define REGISTRAR_REGISTRAR_ADD_NAME_DESC   "add new registrar (make a copy of REG-FRED_A)"
#define REGISTRAR_REGISTRAR_ADD_ZONE_NAME       "registrar_add_zone"
#define REGISTRAR_REGISTRAR_ADD_ZONE_NAME_DESC  "add access right for registrar to zone"

#define REGISTRAR_ZONE_ADD_HELP_NAME                "zone_add_help"
#define REGISTRAR_ZONE_ADD_HELP_NAME_DESC           "help for zone_add"
#define REGISTRAR_REGISTRAR_ADD_HELP_NAME           "registrar_add_help"
#define REGISTRAR_REGISTRAR_ADD_HELP_NAME_DESC      "help for registrar_add"
#define REGISTRAR_REGISTRAR_ADD_ZONE_HELP_NAME      "registrar_add_zone_help"
#define REGISTRAR_REGISTRAR_ADD_ZONE_HELP_NAME_DESC "help for registrar_add_zone"

#define REGISTRAR_ZONE_FQDN_NAME        "zone_fqdn"
#define REGISTRAR_ZONE_FQDN_NAME_DESC   "fqdn of new zone"

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
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    boost::program_options::options_description *m_options;
    boost::program_options::options_description *m_optionsInvis;
public:
    RegistrarClient();
    RegistrarClient(std::string connstring,
            std::string nsAddr);
    ~RegistrarClient();
    void init(std::string connstring,
            std::string nsAddr,
            Config::Conf &conf);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;
    void show_opts() const;

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
