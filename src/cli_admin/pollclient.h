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

#include <boost/program_options.hpp>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"
#include "baseclient.h"

#include "poll_params.h"

namespace Admin {

class PollClient : public BaseClient {
private:
    DBSharedPtr m_db;
    ccReg::EPP_var m_epp;

    std::string nameservice_context;
    bool poll_list_all;//POLL_LIST_ALL_NAME
    PollListAllArgs poll_list_all_params;
    bool poll_create_statechanges;//POLL_CREATE_STATE_CHANGES_NAME
    PollCreateStatechangesArgs poll_create_statechanges_params;
    bool poll_create_request_fee_messages;
    PollCreateRequestFeeMessagesArgs poll_create_request_fee_messages_params;

    static const struct options m_opts[];
public:
    PollClient()
    : poll_list_all(false)
    , poll_create_statechanges(false)
    { }
    PollClient(
        const std::string &connstring
        , const std::string &nsAddr
        , const std::string& _nameservice_context
        , bool _poll_list_all
        , const PollListAllArgs& _poll_list_all_params
        , bool _poll_create_statechanges
        , const PollCreateStatechangesArgs& _poll_create_statechanges_params
        , bool _poll_create_request_fee_messages
        , const PollCreateRequestFeeMessagesArgs& _poll_create_request_fee_messages_params
        )
    : BaseClient(connstring, nsAddr)
    , nameservice_context(_nameservice_context)
    , poll_list_all(_poll_list_all)
    , poll_list_all_params(_poll_list_all_params)
    , poll_create_statechanges(_poll_create_statechanges)
    , poll_create_statechanges_params(_poll_create_statechanges_params)
    , poll_create_request_fee_messages(_poll_create_request_fee_messages)
    , poll_create_request_fee_messages_params(_poll_create_request_fee_messages_params)
    {
        m_db = connect_DB(connstring
                , std::runtime_error("PollClient db connection failed"));
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
    void create_request_fee_messages();
}; // class PollClient

} // namespace Admin;

#endif // _POLLCLIENT_H_
