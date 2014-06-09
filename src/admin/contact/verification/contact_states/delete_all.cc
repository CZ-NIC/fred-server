#include "src/admin/contact/verification/contact_states/delete_all.h"
#include "src/admin/contact/verification/contact_states/enum.h"
#include "src/fredlib/object_state/cancel_object_state_request_id.h"

// legacy
#include "src/fredlib/object_states.h"

#include <boost/foreach.hpp>

namespace Admin
{
namespace AdminContactVerificationObjectStates
{

    bool conditionally_cancel_final_states(
        Fred::OperationContext& ctx,
        unsigned long long contact_id,
        const Optional<std::string>& name,
        const Optional<std::string>& organization,
        const Optional<std::string>& street1,
        const Optional<std::string>& street2,
        const Optional<std::string>& street3,
        const Optional<std::string>& city,
        const Optional<std::string>& stateorprovince,
        const Optional<std::string>& postalcode,
        const Optional<std::string>& country,
        const Optional<std::string>& telephone,
        const Optional<std::string>& fax,
        const Optional<std::string>& email,
        const Optional<std::string>& notifyemail,
        const Optional<std::string>& vat,
        const Optional<std::string>& ssntype,
        const Optional<std::string>& ssn
    ) {
        if(    name.isset()
            || organization.isset()
            || street1.isset()
            || street2.isset()
            || street3.isset()
            || city.isset()
            || stateorprovince.isset()
            || postalcode.isset()
            || country.isset()
            || telephone.isset()
            || fax.isset()
            || email.isset()
            || notifyemail.isset()
            || vat.isset()
            || ssntype.isset()
            || ssn.isset()
        ) {
            cancel_final_states(ctx, contact_id);

            return true;
        }

        return false;
    }

    static void cancel_states(
        Fred::OperationContext&         _ctx,
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
                 Fred::CancelObjectStateRequestId(
                         _contact_id,
                     object_states_to_erase
                 ).exec(_ctx);
                 _ctx.get_conn().exec("RELEASE SAVEPOINT state_savepoint");
                 _ctx.get_conn().exec("SAVEPOINT state_savepoint");
             } catch(Fred::CancelObjectStateRequestId::Exception& e) {
                 // in case it throws with unknown cause
                 if(!e.is_set_state_not_found()) {
                     throw;
                 } else {
                     _ctx.get_conn().exec("ROLLBACK TO state_savepoint");
                 }
             }
         }
    }

    void cancel_all_states(Fred::OperationContext& _ctx, unsigned long long _contact_id) {
        cancel_states(_ctx, _contact_id, Admin::AdminContactVerificationObjectStates::get_all());
    }

    void cancel_final_states(Fred::OperationContext& _ctx, unsigned long long _contact_id) {
        cancel_states(_ctx, _contact_id, Admin::AdminContactVerificationObjectStates::get_final());
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
                Admin::AdminContactVerificationObjectStates::get_final()
            ) {
                Fred::cancel_object_state(contact_id, object_state);
            }

            return true;
        }
    }
}
}

