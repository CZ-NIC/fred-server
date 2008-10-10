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

#ifndef _OBJECTCLIENT_H_
#define _OBJECTCLIENT_H_

#define OBJECT_SHOW_OPTS_NAME               "object_show_opts"
#define OBJECT_SHOW_OPTS_NAME_DESC          "show all object command line options"
#define OBJECT_NEW_STATE_REQUEST_NAME       "object_new_state_request"
#define OBJECT_NEW_STATE_REQUEST_NAME_DESC  "set request for object state with specified state id"
#define OBJECT_LIST_NAME                    "object_list"
#define OBJECT_LIST_NAME_DESC               "object list description"

#define OBJECT_ID_NAME      "object_id"
#define OBJECT_ID_NAME_DESC "object id"
#define OBJECT_NAME_NAME    "object_name"
#define OBJECT_NAME_NAME_DESC "object name"

#define OBJECT_DEBUG_NAME       "object_delete_debug"
#define OBJECT_DEBUG_NAME_DESC  "object delete debug"

#define OBJECT_UPDATE_STATES_NAME           "object_update_states"
#define OBJECT_UPDATE_STATES_NAME_DESC      "globally update all states of all objects"
#define OBJECT_DELETE_CANDIDATES_NAME       "object_delete_candidates"
#define OBJECT_DELETE_CANDIDATES_NAME_DESC  "delete all objects marked with ``deleteCandidate'' state"
#define OBJECT_REGULAR_PROCEDURE_NAME       "object_regular_procedure"
#define OBJECT_REGULAR_PROCEDURE_NAME_DESC  "shortcut for 2x update_object_states, notify_state changes, " \
    "poll_create_statechanges, object_delete_candidates, poll_create_low_credit, notify_letters_create"
#define OBJECT_NOTIFY_EXCEPT_TYPES_NAME     "notify_except_types"
#define OBJECT_NOTIFY_EXCEPT_TYPES_NAME_DESC "list of notification types ignored in notification"
#define OBJECT_POLL_EXCEPT_TYPES_NAME       "poll_except_types"
#define OBJECT_POLL_EXCEPT_TYPES_NAME_DESC  "list of poll message types ignored in creation (only states now)"
#define OBJECT_DELETE_TYPES_NAME            "object_delete_types"
#define OBJECT_DELETE_TYPES_NAME_DESC       "only this type of object will be delete during mass delete"
#define OBJECT_DELETE_LIMIT_NAME            "object_delete_limit"
#define OBJECT_DELETE_LIMIT_NAME_DESC       "limit for object deleting"

    

#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"
#include "register/register.h"
//#include "corba/mailer_manager.h"

namespace Admin {

class ObjectClient {
private:
    std::string m_connstring;
    std::string m_nsAddr;
    CORBA::Long m_clientId;
    DB m_db;
    Database::Manager *m_dbman;
    ccReg::EPP_var m_epp;

    boost::program_options::options_description *m_options;
    boost::program_options::options_description *m_optionsInvis;
    Config::Conf m_conf;
public:
    ObjectClient();
    ObjectClient(std::string connstring,
            std::string nsAddr);
    ~ObjectClient();
    void init(std::string connstring,
            std::string nsAddr,
            Config::Conf &conf);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;
    void show_opts() const;
    
    int createObjectStateRequest(Register::TID object, unsigned state);
    int deleteObjects(const std::string &typeList);

    int new_state_request();
    void list();
    int update_states();
    int delete_candidates();

    int regular_procedure();
};

} // namespace Admin;

#endif // _OBJECTCLIENT_H_

