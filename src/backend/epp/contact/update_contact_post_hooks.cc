/*
 * Copyright (C) 2016-2022  CZ.NIC, z. s. p. o.
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

#include "src/backend/epp/contact/update_contact_post_hooks.hh"

#include "src/backend/admin/contact/verification/contact_states/delete_all.hh"
#include "src/backend/admin/contact/verification/contact_states/enum.hh"
#include "src/backend/admin/contact/verification/delete_domains_of_invalid_contact.hh"
#include "src/backend/admin/contact/verification/enqueue_check.hh"
#include "src/backend/admin/contact/verification/exceptions.hh"
#include "src/backend/admin/contact/verification/fill_check_queue.hh"
#include "src/backend/admin/contact/verification/list_active_checks.hh"
#include "src/backend/admin/contact/verification/related_records.hh"
#include "src/backend/admin/contact/verification/resolve_check.hh"
#include "src/backend/admin/contact/verification/run_all_enqueued_checks.hh"
#include "src/backend/admin/contact/verification/update_tests.hh"

#include "libfred/object/object_state.hh"
#include "libfred/object/object_type.hh"

#include "libfred/object_state/cancel_object_state_request_id.hh"
#include "libfred/object_state/lock_object_state_request_lock.hh"
#include "libfred/object_state/perform_object_state_request.hh"

#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/copy_contact.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_diff.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"

namespace Epp {
namespace Contact {

namespace {

void conditionally_cancel_contact_verification_states(
        LibFred::OperationContext& _ctx,
        unsigned long long _contact_id)
{
    LibFred::LockObjectStateRequestLock(_contact_id).exec(_ctx);
    LibFred::PerformObjectStateRequest(_contact_id).exec(_ctx);

    const std::string type_of_object = Conversion::Enums::to_db_handle(LibFred::Object_Type::contact);
    const Database::Result contact_change_res = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT (COALESCE(TRIM(ch1.email),'')!=COALESCE(TRIM(ch2.email),'')) OR "
                   "(COALESCE(TRIM(ch1.telephone),'')!=COALESCE(TRIM(ch2.telephone),'')) OR "
                   "(UPPER(COALESCE(TRIM(ch1.name),''))!=UPPER(COALESCE(TRIM(ch2.name),''))) OR "
                   "(COALESCE(TRIM(ch1.organization),'')!=COALESCE(TRIM(ch2.organization),'')) OR "
                   "(COALESCE(TRIM(ch1.street1),'')!=COALESCE(TRIM(ch2.street1),'')) OR "
                   "(COALESCE(TRIM(ch1.street2),'')!=COALESCE(TRIM(ch2.street2),'')) OR "
                   "(COALESCE(TRIM(ch1.street3),'')!=COALESCE(TRIM(ch2.street3),'')) OR "
                   "(COALESCE(TRIM(ch1.city),'')!=COALESCE(TRIM(ch2.city),'')) OR "
                   "(COALESCE(TRIM(ch1.stateorprovince),'')!=COALESCE(TRIM(ch2.stateorprovince),'')) OR "
                   "(COALESCE(TRIM(ch1.postalcode),'')!=COALESCE(TRIM(ch2.postalcode),'')) OR "
                   "(COALESCE(TRIM(ch1.country),'')!=COALESCE(TRIM(ch2.country),'')) "
            "FROM object_registry obr "
            "JOIN history h ON h.next=obr.historyid "
            "JOIN contact_history ch1 ON ch1.historyid=h.next "
            "JOIN contact_history ch2 ON ch2.historyid=h.id "
            "WHERE obr.id=$1::BIGINT AND "
                  "obr.type=get_object_type_id($2::TEXT) AND "
                  "obr.erdate IS NULL",
            // clang-format on
            Database::query_param_list(_contact_id)(type_of_object));
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
            LibFred::StatusList states_to_cancel;
            states_to_cancel.insert(
                    Conversion::Enums::to_db_handle(LibFred::Object_State::conditionally_identified_contact));
            states_to_cancel.insert(
                    Conversion::Enums::to_db_handle(LibFred::Object_State::identified_contact));
            LibFred::CancelObjectStateRequestId(
                    _contact_id,
                    states_to_cancel).exec(_ctx);
        }
        catch (const LibFred::CancelObjectStateRequestId::Exception& e)
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
            LibFred::OperationContext& _ctx,
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
    LibFred::OperationContext& ctx_;
    const std::string name_;
};

} // namespace Epp::Contact::{anonymous}

void update_contact_post_hooks(
        LibFred::OperationContext& _ctx,
        const std::string& _contact_handle,
        const Optional<unsigned long long>& _logd_request_id,
        const bool _epp_update_contact_enqueue_check)
{
    DbSavepoint savepoint(_ctx, "before_update_contact_post_hooks");

    try
    {
        // TODO fredlib_modification - mozna uz obecnejsi pattern, ze jako vstup mam handle a volana implementace po me chce idcko
        const unsigned long long contact_id =
            LibFred::InfoContactByHandle(_contact_handle).exec(_ctx).info_contact_data.id;

        LibFred::PerformObjectStateRequest(contact_id).exec(_ctx);

        conditionally_cancel_contact_verification_states(
                _ctx,
                contact_id);

        LibFred::PerformObjectStateRequest(contact_id).exec(_ctx);
        // admin contact verification Ticket #10935
        if (Fred::Backend::Admin::Contact::Verification::ContactStates::conditionally_cancel_final_states(
                    _ctx,
                    contact_id))
        {
            if (_epp_update_contact_enqueue_check)
            {
                Fred::Backend::Admin::Contact::Verification::enqueue_check_if_no_other_exists(
                        _ctx,
                        contact_id,
                        LibFred::TestsuiteHandle::AUTOMATIC,
                        _logd_request_id);
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
