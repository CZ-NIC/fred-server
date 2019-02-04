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

#include <utility>

#include "src/bin/cli/commonclient.hh"
#include "src/bin/cli/pollclient.hh"
#include "libfred/poll/create_state_messages.hh"
#include "src/deprecated/libfred/poll/create_request_fee_info_messages.hh"
#include "src/bin/corba/logger_client_impl.hh"
#include "libfred/poll/message_type_set.hh"

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/util/cfg/config_handler_decl.hh"

#include <boost/optional.hpp>

namespace Admin {


void
PollClient::runMethod()
{
    if (poll_create_statechanges) { //POLL_CREATE_STATE_CHANGES_NAME
        create_state_changes();
    } else if (poll_create_request_fee_messages) {
        create_request_fee_messages();
    }
}

void
PollClient::create_state_changes()
{
    const std::set<LibFred::Poll::MessageType::Enum> except_types =
        poll_create_statechanges_params.poll_except_types.is_value_set()
        ? Conversion::Enums::Sets::from_config_string<LibFred::Poll::MessageType>(
                poll_create_statechanges_params.poll_except_types.get_value())
        : std::set<LibFred::Poll::MessageType::Enum>();

    const boost::optional<int> limit = poll_create_statechanges_params.poll_limit.is_value_set()
        ? poll_create_statechanges_params.poll_limit.get_value()
        : boost::optional<int>();

    LibFred::OperationContextCreator ctx;
    LibFred::Poll::CreateStateMessages(except_types, limit).exec(ctx);
    ctx.commit_transaction();
}

void
PollClient::create_request_fee_messages()
{
    boost::optional<boost::gregorian::date> period_to;
    if(poll_create_request_fee_messages_params.poll_period_to.is_value_set()) {
        period_to = from_simple_string(
            poll_create_request_fee_messages_params.poll_period_to.get_value()
        );

        if (period_to->is_special())
        {
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

    LibFred::Logger::LoggerCorbaClientImpl cl;

    // just to compile it for now
    LibFred::OperationContextCreator ctx;
    const unsigned long long zone_id = 2; // bogus argument
    LibFred::Poll::create_request_fee_info_messages(ctx, cl, zone_id, period_to, "Europe/Prague");
    ctx.commit_transaction();
}

} // namespace Admin;

