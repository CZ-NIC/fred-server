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
#include "src/backend/admin/contact/verification/contact_states/enum.hh"
#include "src/deprecated/libfred/db_settings.hh"
#include "libfred/object_state/cancel_object_state_request_id.hh"

// legacy
#include "src/deprecated/libfred/object_states.hh"

#include <boost/foreach.hpp>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {
namespace ContactStates {

    bool conditionally_cancel_final_states(
        LibFred::OperationContext& ctx,
        unsigned long long contact_id,
        bool name_changed,
        bool organization_changed,
        bool street1_changed,
        bool street2_changed,
        bool street3_changed,
        bool city_changed,
        bool stateorprovince_changed,
        bool postalcode_changed,
        bool country_changed,
        bool telephone_changed,
        bool fax_changed,
        bool email_changed,
        bool notifyemail_changed,
        bool vat_changed,
        bool ssntype_changed,
        bool ssn_changed
    ) {
        if(    name_changed
            || organization_changed
            || street1_changed
            || street2_changed
            || street3_changed
            || city_changed
            || stateorprovince_changed
            || postalcode_changed
            || country_changed
            || telephone_changed
            || fax_changed
            || email_changed
            || notifyemail_changed
            || vat_changed
            || ssntype_changed
            || ssn_changed
        ) {
            cancel_final_states(ctx, contact_id);

            return true;
        }

        return false;
    }

    static void cancel_states(
        LibFred::OperationContext&         _ctx,
        unsigned long long              _contact_id,
        const std::vector<std::string>&  _states
    ) {
        _ctx.get_conn().exec("SAVEPOINT state_savepoint");

         // cancel one state at a time because when exception is thrown, all changes would be ROLLBACKed
         BOOST_FOREACH(
             const std::string& object_state,
             _states
         ) {
             std::set<std::string> object_states_to_erase = boost::assign::list_of(object_state);
             try {
                 LibFred::CancelObjectStateRequestId(
                         _contact_id,
                     object_states_to_erase
                 ).exec(_ctx);
                 _ctx.get_conn().exec("RELEASE SAVEPOINT state_savepoint");
                 _ctx.get_conn().exec("SAVEPOINT state_savepoint");
             } catch(LibFred::CancelObjectStateRequestId::Exception& e) {
                 // in case it throws with unknown cause
                 if(!e.is_set_state_not_found()) {
                     throw;
                 } else {
                     _ctx.get_conn().exec("ROLLBACK TO state_savepoint");
                 }
             }
         }
    }

    void cancel_all_states(LibFred::OperationContext& _ctx, unsigned long long _contact_id) {
        cancel_states(_ctx, _contact_id, get_all());
    }

    void cancel_final_states(LibFred::OperationContext& _ctx, unsigned long long _contact_id) {
        cancel_states(_ctx, _contact_id, get_final());
    }

    bool conditionally_cancel_final_states_legacy(
        unsigned long long contact_id
    ) {
        Database::Connection db_conn = Database::Manager::acquire();

        // is there any change?
        Database::Result result = db_conn.exec_params(
            "SELECT "
                        "(c.name            IS DISTINCT FROM c_h_before.name) "
                    "OR  (c.organization    IS DISTINCT FROM c_h_before.organization) "
                    "OR  (c.street1         IS DISTINCT FROM c_h_before.street1) "
                    "OR  (c.street2         IS DISTINCT FROM c_h_before.street2) "
                    "OR  (c.street3         IS DISTINCT FROM c_h_before.street3) "
                    "OR  (c.city            IS DISTINCT FROM c_h_before.city) "
                    "OR  (c.stateorprovince IS DISTINCT FROM c_h_before.stateorprovince) "
                    "OR  (c.postalcode      IS DISTINCT FROM c_h_before.postalcode) "
                    "OR  (c.country         IS DISTINCT FROM c_h_before.country) "
                    "OR  (c.telephone       IS DISTINCT FROM c_h_before.telephone) "
                    "OR  (c.fax             IS DISTINCT FROM c_h_before.fax) "
                    "OR  (c.email           IS DISTINCT FROM c_h_before.email) "
                    "OR  (c.notifyemail     IS DISTINCT FROM c_h_before.notifyemail) "
                    "OR  (c.vat             IS DISTINCT FROM c_h_before.vat) "
                    "OR  (c.ssn             IS DISTINCT FROM c_h_before.ssn) "
                    "OR  (c.ssntype         IS DISTINCT FROM c_h_before.ssntype) "
                "FROM object_registry AS o_r "
                    "JOIN contact AS c USING( id ) "
                    "JOIN history AS h_before ON o_r.historyid = h_before.next "
                    "JOIN contact_history AS c_h_before ON h_before.id = c_h_before.historyid "
                "WHERE o_r.id = $1::bigint ",
            Database::query_param_list(contact_id)
        );

        if( static_cast<bool>(result[0][0]) == false ) {
            return false;

        } else {
            BOOST_FOREACH(
                const std::string& object_state,
                get_final()
            ) {
                LibFred::cancel_object_state_request(contact_id, object_state);
            }

            return true;
        }
    }

    bool conditionally_cancel_final_states(
        LibFred::OperationContext& ctx,
        unsigned long long contact_id
    ) {
        // is there any change?
        Database::Result result = ctx.get_conn().exec_params(
            "SELECT "
                        "(c.name            IS DISTINCT FROM c_h_before.name) "
                    "OR  (c.organization    IS DISTINCT FROM c_h_before.organization) "
                    "OR  (c.street1         IS DISTINCT FROM c_h_before.street1) "
                    "OR  (c.street2         IS DISTINCT FROM c_h_before.street2) "
                    "OR  (c.street3         IS DISTINCT FROM c_h_before.street3) "
                    "OR  (c.city            IS DISTINCT FROM c_h_before.city) "
                    "OR  (c.stateorprovince IS DISTINCT FROM c_h_before.stateorprovince) "
                    "OR  (c.postalcode      IS DISTINCT FROM c_h_before.postalcode) "
                    "OR  (c.country         IS DISTINCT FROM c_h_before.country) "
                    "OR  (c.telephone       IS DISTINCT FROM c_h_before.telephone) "
                    "OR  (c.fax             IS DISTINCT FROM c_h_before.fax) "
                    "OR  (c.email           IS DISTINCT FROM c_h_before.email) "
                    "OR  (c.notifyemail     IS DISTINCT FROM c_h_before.notifyemail) "
                    "OR  (c.vat             IS DISTINCT FROM c_h_before.vat) "
                    "OR  (c.ssn             IS DISTINCT FROM c_h_before.ssn) "
                    "OR  (c.ssntype         IS DISTINCT FROM c_h_before.ssntype) "
                "FROM object_registry AS o_r "
                    "JOIN contact AS c USING( id ) "
                    "JOIN history AS h_before ON o_r.historyid = h_before.next "
                    "JOIN contact_history AS c_h_before ON h_before.id = c_h_before.historyid "
                "WHERE o_r.id = $1::bigint ",
            Database::query_param_list(contact_id)
        );

        if( static_cast<bool>(result[0][0]) == false ) {
            return false;

        } else {
            const std::vector<std::string> final_states = get_final();
            try {
                LibFred::CancelObjectStateRequestId(
                    contact_id,
                    std::set<std::string>(final_states.begin(), final_states.end())
                ).exec(ctx);
            } catch(const LibFred::CancelObjectStateRequestId::Exception& ex) {
                if(ex.is_set_state_not_found() && !ex.is_set_object_id_not_found()) {
                    /* swallow it - means that the state just wasn't set and nothing else */
                } else {
                    throw;
                }
            }

            return true;
        }
    }
} // namspace Fred::Backend::Admin::Contact::Verification::ContactStates
} // namspace Fred::Backend::Admin::Contact::Verification
} // namspace Fred::Backend::Admin::Contact
} // namspace Fred::Backend::Admin
} // namspace Fred::Backend
} // namspace Fred
