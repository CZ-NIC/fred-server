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

#ifndef _NOTIFYCLIENT_H_
#define _NOTIFYCLIENT_H_

#define NOTIFY_SHOW_OPTS_NAME           "notify_show_opts"
#define NOTIFY_SHOW_OPTS_NAME_DESC      "show all notify command line options"
#define NOTIFY_STATE_CHANGES_NAME       "notify_state_changes"
#define NOTIFY_STATE_CHANGES_NAME_DESC  "send emails to contacts abou object state changes"
#define NOTIFY_LETTERS_CREATE_NAME      "notify_letters_create"
#define NOTIFY_LETTERS_CREATE_NAME_DESC "generate pdf with domain registration warning"

#define NOTIFY_EXCEPT_TYPES_NAME        "notify_except_types"
#define NOTIFY_EXCEPT_TYPES_NAME_DESC   "list of notification types ignored in notification"
#define NOTIFY_USE_HISTORY_TABLES_NAME  "notify_use_history_tables"
#define NOTIFY_USE_HISTORY_TABLES_NAME_DESC "slower queries into historic tables, but can handle deleted objects"
#define NOTIFY_LIMIT_NAME               "notify_limit"
#define NOTIFY_LIMIT_NAME_DESC          "limit for nubmer of emails generated in one pass (0=no limit)"
#define NOTIFY_DEBUG_NAME               "notify_debug"
#define NOTIFY_DEBUG_NAME_DESC          "debug"

#include <boost/program_options.hpp>
#include <iostream>


#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"
#include "register/register.h"
#include "baseclient.h"

namespace Admin {

class NotifyClient : public BaseClient {
private:
    CORBA::Long m_clientId;
    DB m_db;
    boost::program_options::variables_map m_varMap;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    static const struct options m_opts[];
public:
    NotifyClient()
    { }
    NotifyClient(
            const std::string &connstring,
            const std::string &nsAddr,
            const Config::Conf &conf):
        BaseClient(connstring, nsAddr),
        m_conf(conf)
    {
        m_db.OpenDatabase(connstring.c_str());
    }
    ~NotifyClient()
    { }

    static const struct options *getOpts();
    static int getOptsCount();
    void runMethod();

    void show_opts();
    void state_changes();
    void letters_create();
}; // class NotifyClient

} // namespace Admin;

#endif // _NOTIFYCLIENT_H_

