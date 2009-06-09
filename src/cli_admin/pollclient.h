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

#ifndef _POLLCLIENT_H_
#define _POLLCLIENT_H_

#define POLL_SHOW_OPTS_NAME         "poll_show_opts"
#define POLL_SHOW_OPTS_NAME_DESC    "show all poll command line options"
#define POLL_LIST_ALL_NAME          "poll_list_all"
#define POLL_LIST_ALL_NAME_DESC     "list all poll messages"
#define POLL_LIST_NEXT_NAME         "poll_list_next"
#define POLL_LIST_NEXT_NAME_DESC    "list next message for given registrar id"
#define POLL_LIST_NEXT_HANDLE_NAME  "poll_list_next_handle"
#define POLL_LIST_NEXT_HANDLE_NAME_DESC "list next message for given registrar handle"
#define POLL_SET_SEEN_NAME          "poll_set_seen"
#define POLL_SET_SEEN_NAME_DESC     "set given message as seen"

#define POLL_CREATE_STATE_CHANGES_NAME          "poll_create_statechanges"
#define POLL_CREATE_STATE_CHANGES_NAME_DESC     "create messages for state changes"
#define POLL_CREATE_STATE_CHANGES_2_NAME        "poll_create_state_changes"
#define POLL_CREATE_STATE_CHANGES_2_NAME_DESC   "create messages for state changes"
#define POLL_CREATE_LOW_CREDIT_NAME             "poll_create_lowcredit"
#define POLL_CREATE_LOW_CREDIT_NAME_DESC        "create messages for low credit"
#define POLL_CREATE_LOW_CREDIT_2_NAME           "poll_create_low_credit"
#define POLL_CREATE_LOW_CREDIT_2_NAME_DESC      "create messages for low credit"

#define POLL_TYPE_NAME              "poll_type"
#define POLL_TYPE_NAME_DESC         "set filter for poll type"
#define POLL_REGID_NAME             "poll_regid"
#define POLL_REGID_NAME_DESC        "set filter for registrar id"
#define POLL_NONSEEN_NAME           "poll_nonseen"
#define POLL_NONSEEN_NAME_DESC      "set filter for non seen messages"
#define POLL_NONEX_NAME             "poll_nonex"
#define POLL_NONEX_NAME_DESC        "set filter for non expired messages"
#define POLL_DEBUG_NAME             "poll_debug"
#define POLL_DEBUG_NAME_DESC        "don't do actually anything, just list xml with values"
#define POLL_EXCEPT_TYPES_NAME      "poll_except_types"
#define POLL_EXCEPT_TYPES_NAME_DESC "list of poll messages types ignored in creation (only states now)"
#define POLL_LIMIT_NAME             "poll_limit"
#define POLL_LIMIT_NAME_DESC        "limit for number of messages generated in one pass (0=no limit)"

#include <boost/program_options.hpp>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"
#include "baseclient.h"

namespace Admin {

class PollClient : public BaseClient {
private:
    CORBA::Long m_clientId;
    DB m_db;
    ccReg::EPP_var m_epp;
    Config::Conf m_conf;

    static const struct options m_opts[];
public:
    PollClient()
    { }
    PollClient(
            const std::string &connstring,
            const std::string &nsAddr,
            const Config::Conf &conf):
        BaseClient(connstring, nsAddr),
        m_conf(conf)
    {
        m_db.OpenDatabase(connstring.c_str());
    }
    ~PollClient()
    { }

    static const struct options *getOpts();
    static int getOptsCount();
    void runMethod();

    void show_opts();
    void list_all();
    void list_next();
    void set_seen();
    void create_state_changes();
    void create_low_credit();
}; // class PollClient

} // namespace Admin;

#endif // _POLLCLIENT_H_
