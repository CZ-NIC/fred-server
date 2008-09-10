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

#ifndef _POLLCLIENT_H_
#define _POLLCLIENT_H_

#define POLL_LIST_ALL_NAME          "poll-list-all"
#define POLL_LIST_NEXT_NAME         "poll-list-next"
#define POLL_SET_SEEN_NAME          "poll-set-seen"
#define POLL_CREATE_STATE_CHANGES_NAME   "poll-create-state-changes"
#define POLL_CREATE_LOW_CREDIT_NAME "poll-create-low-credit"

#define POLL_TYPE_NAME              "type"
#define POLL_REGID_NAME             "regid"
#define POLL_NONSEEN_NAME           "nonseen"
#define POLL_NONEX_NAME             "nonex"
#define POLL_DEBUG_NAME             "debug"
#define POLL_DRY_RUN_NAME           "dry-run"
#define POLL_EXCEPT_TYPES_NAME      "except-types"
#define POLL_LIMIT_NAME             "poll-limit"

#include <boost/program_options.hpp>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"

namespace Admin {

class PollClient {
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
    PollClient();
    PollClient(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);
    ~PollClient();
    void init(std::string connstring,
            std::string nsAddr,
            boost::program_options::variables_map varMap);

    boost::program_options::options_description *getVisibleOptions() const;
    boost::program_options::options_description *getInvisibleOptions() const;

    int list_all();
    int list_next();
    int set_seen();
    int create_state_changes();
    int create_low_credit();
};

} // namespace Admin;

#endif // _POLLCLIENT_H_
