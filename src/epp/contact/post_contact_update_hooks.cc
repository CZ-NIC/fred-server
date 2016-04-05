#include "src/epp/contact/post_contact_update_hooks.h"

#include <admin/admin_contact_verification.h>
#include <fredlib/contact.h>
#include "src/admin/contact/verification/contact_states/delete_all.h"
#include "src/admin/contact/verification/contact_states/enum.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/cancel_object_state_request_id.h"
#include "src/fredlib/object_state/object_state_name.h"

namespace Epp {

static void conditionally_cancel_contact_verification_states(Fred::OperationContext& _ctx, const std::string& _contact_handle) {
    const unsigned long long contact_id = Fred::InfoContactByHandle(_contact_handle).exec(_ctx).info_contact_data.id;

    Fred::LockObjectStateRequestLock(contact_id).exec(_ctx);
    Fred::PerformObjectStateRequest(contact_id).exec(_ctx);

    const Database::Result contact_change_res = _ctx.get_conn().exec_params(
        "SELECT "
        "("
            "(COALESCE(ch1.email,'')            != COALESCE(ch2.email,'')) OR "
            "(COALESCE(ch1.telephone,'')        != COALESCE(ch2.telephone,'')) OR "
            "(COALESCE(ch1.name,'')             != COALESCE(ch2.name,'')) OR "
            "(COALESCE(ch1.organization,'')     != COALESCE(ch2.organization,'')) OR "
            "(COALESCE(ch1.street1,'')          != COALESCE(ch2.street1,'')) OR "
            "(COALESCE(ch1.street2,'')          != COALESCE(ch2.street2,'')) OR "
            "(COALESCE(ch1.street3,'')          != COALESCE(ch2.street3,'')) OR "
            "(COALESCE(ch1.city,'')             != COALESCE(ch2.city,'')) OR "
            "(COALESCE(ch1.stateorprovince,'')  != COALESCE(ch2.stateorprovince,'')) OR "
            "(COALESCE(ch1.postalcode,'')       != COALESCE(ch2.postalcode,'')) OR "
            "(COALESCE(ch1.country,'')          != COALESCE(ch2.country,'')) "
        ") AS is_different "
        "FROM object_registry oreg "
            "JOIN contact_history ch1 ON ch1.historyid = oreg.historyid "
            "JOIN history h ON h.next = ch1.historyid "
            "JOIN contact_history ch2 ON ch2.historyid = h.id "
            "JOIN contact c ON c.id = oreg.id "
        "WHERE "
            "oreg.name = upper($1::text) "
            "AND oreg.erdate IS NULL ",
        Database::query_param_list(_contact_handle)
    );
    if (contact_change_res.size() != 1) {
        throw std::runtime_error("unable to get contact data difference");
    }

    if( static_cast<bool>(contact_change_res[0]["is_different"]) ) {
        try {
            Fred::CancelObjectStateRequestId(
                contact_id,
                boost::assign::list_of
                    (Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT)
                    (Fred::ObjectState::IDENTIFIED_CONTACT)
            ).exec(_ctx);
        } catch(const Fred::CancelObjectStateRequestId::Exception& ex) {
            if(ex.is_set_state_not_found() && !ex.is_set_object_id_not_found()) {
                /* swallow it - means that the state just wasn't set and nothing else */
            } else {
                throw;
            }
        }
    }
}

void post_contact_update_hooks(
    Fred::OperationContext& _ctx,
    const std::string& _contact_handle,
    const unsigned long long _logd_requst_id,
    const bool _epp_update_contact_enqueue_check
) {
    _ctx.get_conn().exec("SAVEPOINT before_post_contact_update_hooks");

    try {
        // TODO fredlib_modification - mozna uz obecnejsi pattern, ze jako vstup mam handle a volana implementace po me chce idcko
        const unsigned long long contact_id = Fred::InfoContactByHandle(_contact_handle).exec(_ctx).info_contact_data.id;

        Fred::PerformObjectStateRequest(contact_id).exec(_ctx);

        conditionally_cancel_contact_verification_states(_ctx, _contact_handle);

        Fred::PerformObjectStateRequest(contact_id).exec(_ctx);
        // admin contact verification Ticket #10935
        if(Admin::AdminContactVerificationObjectStates::conditionally_cancel_final_states(_ctx, contact_id)) {
            if(_epp_update_contact_enqueue_check) {
                Admin::enqueue_check_if_no_other_exists(_ctx, contact_id, Fred::TestsuiteHandle::AUTOMATIC, _logd_requst_id);
            }
        }

    } catch (...) {
        _ctx.get_conn().exec("ROLLBACK TO before_post_contact_update_hooks");

        throw;
    }
}

}
