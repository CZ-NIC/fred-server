/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#include "src/libfred/registrable_object/contact/undisclose_address.hh"

#include "src/libfred/object/object_states_info.hh"
#include "src/libfred/poll/create_poll_message.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrable_object/contact/update_contact.hh"

#include <string>

namespace LibFred {
namespace Contact {

void undisclose_address(LibFred::OperationContext& _ctx, unsigned long long _contact_id, const std::string& _registrar_handle)
{
    if (LibFred::InfoContactById(_contact_id).exec(_ctx).info_contact_data.discloseaddress)
    {
        LibFred::UpdateContactById update_contact(_contact_id, _registrar_handle);
        const unsigned long long history_id = update_contact.set_discloseaddress(false).exec(_ctx); // #21767
        LibFred::Poll::CreatePollMessage<LibFred::Poll::MessageType::update_contact>()
                .exec(_ctx, history_id);
    }
}

namespace {

bool are_preconditions_met_for_undisclose_address(LibFred::OperationContext& _ctx, unsigned long long _contact_id)
{
    const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(_contact_id).exec(_ctx));

    return (
            states.presents(LibFred::Object_State::identified_contact) ||
            states.presents(LibFred::Object_State::validated_contact)
           )
           &&
           LibFred::InfoContactById(_contact_id).exec(_ctx).info_contact_data.organization.get_value_or("").empty();
}

} // namespace Fred::Util::{anonymous}

void undisclose_address_async(unsigned long long _contact_id, const std::string& _registrar_handle)
{
        LibFred::OperationContextCreator ctx;

        if (are_preconditions_met_for_undisclose_address(ctx, _contact_id))
        {
            ctx.get_log().info(boost::format("processing async undisclose address of contact %1%: preconditions met, undisclosing address") % _contact_id);
            undisclose_address(ctx, _contact_id, _registrar_handle);
        }
        else
        {
            ctx.get_log().info(boost::format("processing async undisclose address of contact %1%: preconditions not met, doing nothing") % _contact_id);
        }

        ctx.commit_transaction();
}

} // namespace LibFred::Contact
} // namespace LibFred
