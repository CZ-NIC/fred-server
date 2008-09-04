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

#ifndef _CONTACTCLIENT_H_
#define _CONTACTCLIENT_H_

#include <boost/program_options.hpp>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"

#define CONTACT_INFO_NAME               "contact-info"
#define CONTACT_INFO_NAME_DESC          "contact info"
#define CONTACT_INFO2_NAME              "contact-info2"
#define CONTACT_INFO2_NAME_DESC         "contact info 2"
#define CONTACT_LIST_NAME               "contact-list"
#define CONTACT_LIST_NAME_DESC          "list of all contacts (via filters)"
#define CONTACT_LIST_PLAIN_NAME         "contact-list-plain"
#define CONTACT_LIST_PLAIN_NAME_DESC    "list of all contacts (via epp_impl)"
#define CONTACT_LIST_HELP_NAME          "contact-list-help"
#define CONTACT_LIST_HELP_NAME_DESC     "help for contact list"

namespace Admin {

class ContactClient {
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
    ContactClient();
    ContactClient(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);
    ~ContactClient();
    void init(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;

    void list();
    int info();
    int info2();

    void list_help();
};

} // namespace Admin;


#endif // _CONTACTCLIENT_H_
