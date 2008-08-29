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

#define OBJECT_NEW_STATE_REQUEST_NAME   "object-new-state-request"
#define OBJECT_LIST_NAME                "object-list"
#define OBJECT_UPDATE_STATES_NAME       "object-update-states"
#define OBJECT_DELETE_CANDIDATES_NAME   "object-delete-candidates"

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
    boost::program_options::variables_map m_varMap;
    ccReg::EPP_var m_epp;

    boost::program_options::options_description *m_options;
    boost::program_options::options_description *m_optionsInvis;
public:
    ObjectClient();
    ObjectClient(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);
    ~ObjectClient();
    void init(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;
    
    int createObjectStateRequest(Register::TID object, unsigned state);
    int deleteObjects(const std::string &typeList);

    int new_state_request();
    int list();
    int update_states();
    int delete_candidates();

};

} // namespace Admin;

#endif // _OBJECTCLIENT_H_

