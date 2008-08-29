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

#ifndef _DOMAINCLIENT_H_
#define _DOMAINCLIENT_H_

#include <iostream>
#include <boost/program_options.hpp>
#include "old_utils/dbsql.h"

#include "old_utils/log.h"
#include "old_utils/conf.h"
#include "register/register.h"
#include "corba/admin/admin_impl.h"
#include "corba/mailer_manager.h"

#define DOMAIN_CREATE_NAME          "domain-create"
#define DOMAIN_UPDATE_NAME          "domain-update"
#define DOMAIN_LIST_NAME            "domain-list"
#define DOMAIN_LIST_PLAIN_NAME      "domain-list-plain"
#define DOMAIN_INFO_NAME            "domain-info"


#define DOMAIN_CREATE_HELP_NAME     "domain-create-help"
#define DOMAIN_UPDATE_HELP_NAME     "domain-update-help"

#define DOMAIN_REGISTRANT_NAME      "registrant"
#define DOMAIN_NSSET_NAME           "nsset"
#define DOMAIN_KEYSET_NAME          "keyset"
#define DOMAIN_PERIOD_NAME          "period"

namespace Admin {

class DomainClient {
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
    DomainClient();
    DomainClient(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);
    ~DomainClient();
    void init(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;

    int domain_list();

    int domain_list_plain();

    int domain_create();

    int domain_update();

    int domain_info();

    void domain_update_help();
    void domain_create_help();
};

} // namespace Admin;

#endif // _DOMAINCLIENT_H_
