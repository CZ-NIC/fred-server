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

#ifndef _NSSETCLIENT_H_
#define _NSSETCLIENT_H_

#include <boost/program_options.hpp>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"
#include "register/register.h"

#define NSSET_LIST_NAME             "nsset-list"
#define NSSET_LIST_NAME_DESC        "List all nsset objects"
#define NSSET_LIST_HELP_NAME        "nsset-list-help"
#define NSSET_LIST_HELP_NAME_DESC   "Help for nsset list"

#define NSSET_HOST_FQDN_NAME        "host-fqdn"
#define NSSET_HOST_FQDN_NAME_DESC   "filter records with specific host FQDN"
#define NSSET_HOST_IP_NAME          "host-ip"
#define NSSET_HOST_IP_NAME_DESC     "filter records with specific host ip address"

namespace Admin {

class NssetClient {
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
    NssetClient();
    NssetClient(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);
    ~NssetClient();
    void init(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;

    int list();
    void list_help();
};

} // namespace Admin;

#endif // _NSSETCLIENT_H_
