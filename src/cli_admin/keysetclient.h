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

#ifndef _KEYSETCLIENT_H_
#define _KEYSETCLIENT_H_

#include <iostream>
#include <boost/program_options.hpp>
#include "old_utils/dbsql.h"

#include "old_utils/log.h"
#include "old_utils/conf.h"
#include "register/register.h"
#include "corba/admin/admin_impl.h"
#include "corba/mailer_manager.h"

#define KEYSET_LIST_NAME            "keyset-list"
#define KEYSET_LIST_PLAIN_NAME      "keyset-list-plain"
#define KEYSET_CHECK_NAME           "keyset-check"
#define KEYSET_SEND_AUTH_INFO_NAME  "keyset-send-auth-info"
#define KEYSET_TRANSFER_NAME        "keyset-transfer"
#define KEYSET_UPDATE_NAME          "keyset-update"
#define KEYSET_DELETE_NAME          "keyset-delete"
#define KEYSET_CREATE_NAME          "keyset-create"
#define KEYSET_CREATE2_NAME         "keyset-create2"
#define KEYSET_INFO_NAME            "keyset-info"
#define KEYSET_INFO2_NAME           "keyset-info2"

#define KEYSET_DSRECORDS_NAME       "dsrecords"
#define KEYSET_DSREC_ADD_NAME       "dsrec-add"
#define KEYSET_DSREC_REM_NAME       "dsrec-rem"

#define KEYSET_UPDATE_HELP_NAME     "keyset-update-help"
#define KEYSET_CREATE_HELP_NAME     "keyset-create-help"
#define KEYSET_DELETE_HELP_NAME     "keyset-delete-help"
#define KEYSET_INFO_HELP_NAME       "keyset-info-help"
#define KEYSET_CHECK_HELP_NAME      "keyset-check-help"

namespace Admin {

class KeysetClient {
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
    KeysetClient();
    KeysetClient(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);
    ~KeysetClient();
    void init(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;

    int keyset_list();

    int keyset_list_plain();

    int keyset_check();

    int keyset_send_auth_info();

    int keyset_transfer();

    int keyset_update();

    int keyset_delete();

    int keyset_create();

    int keyset_info();

    int keyset_info2();

    void keyset_create_help();
    void keyset_update_help();
    void keyset_delete_help();
    void keyset_info_help();
    void keyset_check_help();
};

} // namespace Admin;

#endif // _KEYSETCLIENT_H_
