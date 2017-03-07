#include "src/epp/contact/update_contact_post_hooks.h"

#include "src/admin/contact/verification/contact_states/delete_all.h"
#include "src/admin/contact/verification/contact_states/enum.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/object_state/cancel_object_state_request_id.h"
#include "src/fredlib/object_state/lock_object_state_request_lock.h"
#include "src/fredlib/object_state/perform_object_state_request.h"

#include <admin/admin_contact_verification.h>
#include <fredlib/contact.h>

namespace Epp {
namespace Contact {

namespace {

void conditionally_cancel_contact_verification_states(
        Fred::OperationContext& _ctx,
        unsigned long long _contact_id)
{
    Fred::LockObjectStateRequestLock(_contact_id).exec(_ctx);
    Fred::PerformObjectStateRequest(_contact_id).exec(_ctx);

    const std::string type_of_object = Conversion::Enums::to_db_handle(Fred::Object_Type::contact);
    const Database::Result contact_change_res = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT (COALESCE(ch1.email,'')!=COALESCE(ch2.email,'')) OR "
                   "(COALESCE(ch1.telephone,'')!=COALESCE(ch2.telephone,'')) OR "
                   "(COALESCE(ch1.name,'')!=COALESCE(ch2.name,'')) OR "
                   "(COALESCE(ch1.organization,'')!=COALESCE(ch2.organization,'')) OR "
                   "(COALESCE(ch1.street1,'')!=COALESCE(ch2.street1,'')) OR "
                   "(COALESCE(ch1.street2,'')!=COALESCE(ch2.street2,'')) OR "
                   "(COALESCE(ch1.street3,'')!=COALESCE(ch2.street3,'')) OR "
                   "(COALESCE(ch1.city,'')!=COALESCE(ch2.city,'')) OR "
                   "(COALESCE(ch1.stateorprovince,'')!=COALESCE(ch2.stateorprovince,'')) OR "
                   "(COALESCE(ch1.postalcode,'')!=COALESCE(ch2.postalcode,'')) OR "
                   "(COALESCE(ch1.country,'')!=COALESCE(ch2.country,'')) "
            "FROM object_registry obr "
            "JOIN history h ON h.next=obr.historyid "
            "JOIN contact_history ch1 ON ch1.historyid=h.next "
            "JOIN contact_history ch2 ON ch2.historyid=h.id "
            "WHERE obr.id=$1::BIGINT AND "
                  "obr.type=get_object_type_id($2::TEXT) AND "
                  "obr.erdate IS NULL",
            // clang-format on
            Database::query_param_list(_contact_id)
                (type_of_object));
    if (contact_change_res.size() != 1)
    {
        throw std::runtime_error(
                contact_change_res.size() == 0 ? "unable to get contact data difference"
                                               : "ambiguous contact data");
    }

    if (static_cast<bool>(contact_change_res[0][0]))
    {
        try
        {
            Fred::StatusList states_to_cancel;
            states_to_cancel.insert(
                    Conversion::Enums::to_db_handle(
                            Fred::Object_State::conditionally_identified_contact));
            states_to_cancel.insert(
                    Conversion::Enums::to_db_handle(
                            Fred::Object_State::identified_contact));
            Fred::CancelObjectStateRequestId(_contact_id, states_to_cancel).exec(_ctx);
        }
        catch (const Fred::CancelObjectStateRequestId::Exception& e)
        {
            if (!e.is_set_state_not_found() || e.is_set_object_id_not_found())
            {
                throw;
            }
            // swallow it - means that the state just wasn't set and nothing else
        }
    }
}


class DbSavepoint
{
public:


    DbSavepoint(
            Fred::OperationContext& _ctx,
            const std::string& _name)
        : ctx_(_ctx),
          name_(_name)
    {
        ctx_.get_conn().exec("SAVEPOINT " + name_);
    }


    ~DbSavepoint()
    {
    }


    DbSavepoint& release()
    {
        ctx_.get_conn().exec("RELEASE SAVEPOINT " + name_);
        return *this;
    }


    DbSavepoint& rollback()
    {
        ctx_.get_conn().exec("ROLLBACK TO " + name_);
        return *this;
    }


private:
    Fred::OperationContext& ctx_;
    const std::string name_;
};

} // namespace Epp::Contact::{anonymous}

void update_contact_post_hooks(
        Fred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const Optional<unsigned long long>& _logd_requst_id,
        const bool _epp_update_contact_enqueue_check)
{
    DbSavepoint savepoint(_ctx, "before_update_contact_post_hooks");

    try
    {
        // TODO fredlib_modification - mozna uz obecnejsi pattern, ze jako vstup mam handle a volana implementace po me chce idcko
        const unsigned long long contact_id =
            Fred::InfoContactByHandle(_contact_handle).exec(_ctx).info_contact_data.id;

        Fred::PerformObjectStateRequest(contact_id).exec(_ctx);

        conditionally_cancel_contact_verification_states(_ctx, contact_id);

        Fred::PerformObjectStateRequest(contact_id).exec(_ctx);
        // admin contact verification Ticket #10935
        if (Admin::AdminContactVerificationObjectStates::conditionally_cancel_final_states(_ctx, contact_id))
        {
            if (_epp_update_contact_enqueue_check)
            {
                Admin::enqueue_check_if_no_other_exists(
                        _ctx,
                        contact_id,
                        Fred::TestsuiteHandle::AUTOMATIC,
                        _logd_requst_id);
            }
        }

        savepoint.release();
    }
    catch (...)
    {
        savepoint.rollback();
        savepoint.release();
        throw;
    }
}


} // namespace Epp::Contact
} // namespace Epp
