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

#include "commonclient.h"
#include "pollclient.h"
#include "fredlib/poll.h"
#include "src/corba/logger_client_impl.h"

#include "util/cfg/faked_args.h"
#include "util/cfg/handle_corbanameservice_args.h"
#include "util/cfg/config_handler_decl.h"


namespace Admin {


void
PollClient::runMethod()
{
    if (poll_list_all) {//POLL_LIST_ALL_NAME
        list_all();
    } else if (poll_create_statechanges //POLL_CREATE_STATE_CHANGES_NAME
             ) {
        create_state_changes();
    } else if (poll_create_request_fee_messages) {
        create_request_fee_messages();
    }
}

void
PollClient::list_all()
{

    std::auto_ptr<Fred::Poll::Manager> pollMan(
            Fred::Poll::Manager::create(
                m_db)
            );
    std::auto_ptr<Fred::Poll::List> pmList(pollMan->createList());
    if (poll_list_all_params.poll_type.is_value_set())//POLL_TYPE_NAME
        pmList->setTypeFilter(poll_list_all_params.poll_type.get_value());
    if (poll_list_all_params.registrar_id.is_value_set())//REGISTRAR_ID_NAME
        pmList->setRegistrarFilter(poll_list_all_params.registrar_id.get_value());
    if (poll_list_all_params.poll_nonseen)//POLL_NONSEEN_NAME
        pmList->setNonSeenFilter(true);
    if (poll_list_all_params.poll_nonex)//POLL_NONEX_NAME
        pmList->setNonExpiredFilter(true);
    pmList->reload();
    for (unsigned int i = 0; i < pmList->getCount(); i++) {
        Fred::Poll::Message *msg = pmList->getMessage(i);
        if (msg) {
            msg->textDump(std::cout);
            std::cout << std::endl;
        }
    }
    return;
}

void
PollClient::create_state_changes()
{

    std::auto_ptr<Fred::Poll::Manager> pollMan(
            Fred::Poll::Manager::create(
                m_db)
            );
    std::string exceptTypes("");
    if (poll_create_statechanges_params.poll_except_types.is_value_set()) {//POLL_EXCEPT_TYPES_NAME
        exceptTypes = poll_create_statechanges_params.poll_except_types.get_value();
    }
    int limit = 0;
    if (poll_create_statechanges_params.poll_limit.is_value_set()) {//POLL_LIMIT_NAME
        limit = poll_create_statechanges_params.poll_limit.get_value();
    }
    pollMan->createStateMessages(
            exceptTypes, limit,
            poll_create_statechanges_params.poll_debug ? &std::cout : NULL //POLL_DEBUG_NAME
    );
    return;
}

void
PollClient::create_request_fee_messages()
{
    boost::gregorian::date period_to;
    if(poll_create_request_fee_messages_params.poll_period_to.is_value_set()) {
        period_to = from_simple_string(
            poll_create_request_fee_messages_params.poll_period_to.get_value()
        );

        if(period_to.is_special()) {
            throw std::runtime_error("charge: Invalid poll_msg_period_to.");
        }
    }

    // ORB init
    FakedArgs orb_fa = CfgArgGroups::instance()->fa;

    HandleCorbaNameServiceArgsGrp* ns_args_ptr=CfgArgGroups::instance()->
               get_handler_ptr_by_type<HandleCorbaNameServiceArgsGrp>();

    CorbaContainer::set_instance(orb_fa.get_argc(), orb_fa.get_argv()
           , ns_args_ptr->get_nameservice_host()
           , ns_args_ptr->get_nameservice_port()
           , ns_args_ptr->get_nameservice_context());

    std::auto_ptr<Fred::Logger::LoggerClient> cl(new Fred::Logger::LoggerCorbaClientImpl());

    std::auto_ptr<Fred::Poll::Manager> pollMan(
            Fred::Poll::Manager::create(
                m_db)
            );

    pollMan->createRequestFeeMessages(cl.get(), period_to);

}

} // namespace Admin;

