/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "src/backend/admin/contact/verification/contact_states/delete_all.hh"

#include "libfred/object/object_state.hh"
#include "libfred/object_state/cancel_object_state_request_id.hh"

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {
namespace ContactStates {

namespace {

void cancel_states(LibFred::OperationContext& ctx, unsigned long long contact_id, const std::set<std::string>& states)
{
    try
    {
        LibFred::CancelObjectStateRequestId(contact_id, states).exec(ctx);
        return;
    }
    catch (const LibFred::CancelObjectStateRequestId::Exception& e)
    {
        if (e.is_set_state_not_found() &&
            !e.is_set_object_id_not_found())
        {
            /* swallow it - means that the state just wasn't set and nothing else */
            return;
        }
        throw;
    }
}

}//namespace Fred::Backend::Admin::Contact::Verification::ContactStates::{anonymous}

void cancel_all_states(LibFred::OperationContext& ctx, unsigned long long contact_id)
{
    const std::set<std::string> all_states =
    {
        Conversion::Enums::to_db_handle(LibFred::Object_State::contact_in_manual_verification),
        Conversion::Enums::to_db_handle(LibFred::Object_State::contact_passed_manual_verification),
        Conversion::Enums::to_db_handle(LibFred::Object_State::contact_failed_manual_verification)
    };
    cancel_states(ctx, contact_id, all_states);
}

bool conditionally_cancel_final_states(
    LibFred::OperationContext& ctx,
    unsigned long long contact_id)
{
    // is there any change?
    const Database::Result result = ctx.get_conn().exec_params(
        "SELECT (UPPER(c.name) IS DISTINCT FROM UPPER(c_h_before.name)) OR "
               "(c.organization IS DISTINCT FROM c_h_before.organization) OR "
               "(c.street1 IS DISTINCT FROM c_h_before.street1) OR "
               "(c.street2 IS DISTINCT FROM c_h_before.street2) OR "
               "(c.street3 IS DISTINCT FROM c_h_before.street3) OR "
               "(c.city IS DISTINCT FROM c_h_before.city) OR "
               "(c.stateorprovince IS DISTINCT FROM c_h_before.stateorprovince) OR "
               "(c.postalcode IS DISTINCT FROM c_h_before.postalcode) OR "
               "(c.country IS DISTINCT FROM c_h_before.country) OR "
               "(c.telephone IS DISTINCT FROM c_h_before.telephone) OR "
               "(c.fax IS DISTINCT FROM c_h_before.fax) OR "
               "(c.email IS DISTINCT FROM c_h_before.email) OR "
               "(c.notifyemail IS DISTINCT FROM c_h_before.notifyemail) OR "
               "(c.vat IS DISTINCT FROM c_h_before.vat) OR "
               "(c.ssn IS DISTINCT FROM c_h_before.ssn) OR "
               "(c.ssntype IS DISTINCT FROM c_h_before.ssntype) "
        "FROM object_registry o_r "
        "JOIN contact c USING(id) "
        "JOIN history h_before ON h_before.next=o_r.historyid "
        "JOIN contact_history c_h_before ON c_h_before.historyid=h_before.id "
        "WHERE o_r.id=$1::bigint",
        Database::query_param_list(contact_id));

    if ((result.size() == 0) || !static_cast<bool>(result[0][0]))
    {
        return false;
    }
    const std::set<std::string> final_states =
    {
        Conversion::Enums::to_db_handle(LibFred::Object_State::contact_passed_manual_verification),
        Conversion::Enums::to_db_handle(LibFred::Object_State::contact_failed_manual_verification)
    };
    cancel_states(ctx, contact_id, final_states);
    return true;
}

}//namespace Fred::Backend::Admin::Contact::Verification::ContactStates
}//namespace Fred::Backend::Admin::Contact::Verification
}//namespace Fred::Backend::Admin::Contact
}//namespace Fred::Backend::Admin
}//namespace Fred::Backend
}//namespace Fred
