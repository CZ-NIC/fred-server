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

#ifndef _NSSETCLIENT_H_
#define _NSSETCLIENT_H_

#include <boost/program_options.hpp>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"
#include "register/register.h"
#include "baseclient.h"

#define NSSET_SHOW_OPTS_NAME        "nsset_show_opts"
#define NSSET_SHOW_OPTS_NAME_DESC   "show all nsset command line options parameter"
#define NSSET_LIST_NAME             "nsset_list"
#define NSSET_LIST_NAME_DESC        "List all nsset objects"
#define NSSET_LIST_HELP_NAME        "nsset_list_help"
#define NSSET_LIST_HELP_NAME_DESC   "Help for nsset list"

#define NSSET_HOST_FQDN_NAME        "host_fqdn"
#define NSSET_HOST_FQDN_NAME_DESC   "filter records with specific host FQDN"
#define NSSET_HOST_IP_NAME          "host_ip"
#define NSSET_HOST_IP_NAME_DESC     "filter records with specific host ip address"

namespace Admin {

class NssetClient : public BaseClient {
private:
    CORBA::Long m_clientId;
    DB m_db;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    static const struct options m_opts[];
public:
    NssetClient()
    { }
    NssetClient(
            const std::string &connstring,
            const std::string &nsAddr,
            const Config::Conf &conf):
        BaseClient(connstring, nsAddr),
        m_conf(conf)
    {
        m_db.OpenDatabase(connstring.c_str());
    }
    ~NssetClient()
    { }

    static const struct options *getOpts();
    static int getOptsCount();
    void runMethod();

    void show_opts();
    void list();
    void list_help();
}; // class NssetClient

} // namespace Admin;

#endif // _NSSETCLIENT_H_
