/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @poll_client_impl.h
 *  poll client implementation header
 */

#ifndef POLL_CLIENT_IMPL_HH_E60EC9F468F141FDACC2E9A2E83E9131
#define POLL_CLIENT_IMPL_HH_E60EC9F468F141FDACC2E9A2E83E9131
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/bin/cli/handle_adminclientselection_args.hh"
#include "util/log/context.hh"
#include "src/bin/cli/pollclient.hh"


/**
 * \class poll_create_statechanges_impl
 * \brief admin client implementation of poll_create_statechanges
 */
struct poll_create_statechanges_impl
{
  void operator()() const
  {
      Logging::Context ctx("poll_create_statechanges_impl");
      Admin::PollClient poll_client(
        CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
        , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
        , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
        , true //poll_create_statechanges
        , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientPollCreateStatechangesArgsGrp>()->params //PollCreateStatechangesArgs()
        , false
        , PollCreateRequestFeeMessagesArgs()
        );
      poll_client.runMethod();

      return ;
  }
};

/**
 * \class poll_create_request_fee_messages_impl
 * \brief admin client implementation of creation of poll messages related to request count
 * and request fee
 */
struct poll_create_request_fee_messages_impl
{
    void operator()() const
    {
        Logging::Context ctx("poll_create_request_fee_messages_impl");
        Admin::PollClient poll_client(
          CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDatabaseArgsGrp>()->get_conn_info()
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_host_port()
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>()->get_nameservice_context()
          , false //poll_create_statechanges
          , PollCreateStatechangesArgs()
          , true
          , CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientPollCreateRequestFeeMessagesArgsGrp>()->params
          );
        poll_client.runMethod();

        return ;
    }
};

#endif
