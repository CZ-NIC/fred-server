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

#ifndef _NOTIFYCLIENT_H_
#define _NOTIFYCLIENT_H_

#define NOTIFY_STATE_CHANGES_NAME       "notify-state-changes"
#define NOTIFY_LETTERS_CREATE_NAME      "notify-letters-create"

#define NOTIFY_EXCEPT_TYPES_NAME        "except-types"
#define NOTIFY_USE_HISTORY_TABLES_NAME  "use-history-tables"
#define NOTIFY_LIMIT_NAME               "notify-limit"

#include <iostream>
#include <boost/program_options.hpp>
#include "old_utils/dbsql.h"

#include "old_utils/log.h"
#include "old_utils/conf.h"
#include "register/register.h"
#include "corba/admin/admin_impl.h"
#include "corba/mailer_manager.h"

namespace Admin {

class NotifyClient {
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
    NotifyClient();
    NotifyClient(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);
    ~NotifyClient();
    void init(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;

    int state_changes();
    int letters_create();
};

} // namespace Admin;

#endif // _NOTIFYCLIENT_H_

