#include "src/admin/contact/verification/delete_domains_of_invalid_contact.h"

#include "src/admin/contact/verification/contact_states/enum.h"

#include "src/fredlib/contact/verification/info_check.h"
#include "src/fredlib/contact/verification/enum_testsuite_handle.h"
#include "src/fredlib/contact/verification/enum_check_status.h"

#include <fredlib/domain.h>
#include <fredlib/contact.h>
#include "src/fredlib/poll/create_delete_domain_poll_message.h"
#include "src/fredlib/object_state/get_object_states.h"

#include "util/log/context.h"

#include <set>

#include <boost/foreach.hpp>

namespace  Admin {

    static std::set<unsigned long long> get_owned_domains_locking(
        Fred::OperationContext& _ctx,
        unsigned long long _contact_id);

    static void store_check_poll_message_relation(
        Fred::OperationContext& _ctx,
        const uuid&             _check_handle,
        unsigned long long      _poll_msg_id);

    static bool related_delete_domain_poll_message_exists(
        Fred::OperationContext& _ctx,
        const uuid&             _check_handle);



    void delete_domains_of_invalid_contact(
        Fred::OperationContext& _ctx,
        const uuid&             _check_handle
    ) {
        Logging::Context log("delete_domains_of_invalid_contact");



        try {
            Fred::InfoContactCheckOutput check_info = Fred::InfoContactCheck(_check_handle).exec(_ctx);

            if( check_info.testsuite_handle != Fred::TestsuiteHandle::MANUAL) {
                throw Admin::ExceptionIncorrectTestsuite();
            }

            if( check_info.check_state_history.rbegin()->status_handle != Fred::ContactCheckStatus::FAIL ) {
                throw Admin::ExceptionIncorrectCheckStatus();
            }

            Fred::InfoContactOutput contact_info = Fred::InfoContactHistoryByHistoryid(check_info.contact_history_id).exec(_ctx);

            {
                using namespace AdminContactVerificationObjectStates;
                bool has_state_failed_verification = false;

                BOOST_FOREACH(
                    const Fred::ObjectStateData& state,
                    Fred::GetObjectStates(contact_info.info_contact_data.id).exec(_ctx)
                ) {

                    if(state.state_name == CONTACT_FAILED_MANUAL_VERIFICATION) {
                        has_state_failed_verification = true;
                    } else if(state.state_name == CONTACT_IN_MANUAL_VERIFICATION) {
                        throw Admin::ExceptionIncorrectContactStatus();
                    } else if(state.state_name == CONTACT_PASSED_MANUAL_VERIFICATION) {
                        throw Admin::ExceptionIncorrectContactStatus();
                    }
                }
                if( ! has_state_failed_verification ) {
                    throw Admin::ExceptionIncorrectContactStatus();
                }
            }

            if(related_delete_domain_poll_message_exists(_ctx, _check_handle)) {
                throw Admin::ExceptionDomainsAlreadyDeleted();
            }

            std::set<unsigned long long> domain_ids_to_delete =
                get_owned_domains_locking (
                    _ctx,
                    contact_info.info_contact_data.id
                );

            for(std::set<unsigned long long>::const_iterator it = domain_ids_to_delete.begin();
                it != domain_ids_to_delete.end();
                ++it
            ){
                // beware - need to get info before deleting
                Fred::InfoDomainData info_domain = Fred::InfoDomainById(*it).exec(_ctx).info_domain_data;
                Fred::DeleteDomainById(*it).exec(_ctx);

                store_check_poll_message_relation(
                    _ctx,
                    _check_handle,
                    Fred::Poll::CreateDeleteDomainPollMessage(
                        info_domain.historyid
                    ).exec(_ctx)
                );
            }
        } catch (const Fred::ExceptionUnknownCheckHandle&) {
            throw Admin::ExceptionUnknownCheckHandle();
        }
    }

    std::set<unsigned long long> get_owned_domains_locking(
        Fred::OperationContext& _ctx,
        unsigned long long _contact_id
    ) {
        Database::Result owned_domains_res = _ctx.get_conn().exec_params(
            "SELECT o_r.id AS id_ "
            "   FROM object_registry AS o_r "
            "       JOIN domain AS d USING(id) "
            "   WHERE d.registrant = $1::integer "
            "   FOR UPDATE OF o_r ",
            Database::query_param_list(_contact_id)
        );

        std::set<unsigned long long> result;

        for(Database::Result::Iterator it = owned_domains_res.begin();
            it != owned_domains_res.end();
            ++it
        ) {
            result.insert(static_cast<unsigned long long>( (*it)["id_"]) );
        }

        return result;
    }

    void store_check_poll_message_relation(
        Fred::OperationContext& _ctx,
        const uuid&             _check_handle,
        unsigned long long      _poll_msg_id
    ) {
        _ctx.get_conn().exec_params(
            "INSERT "
            "   INTO contact_check_poll_message_map "
            "   (contact_check_id, poll_message_id) "
            "   VALUES("
            "       (SELECT id FROM contact_check WHERE handle=$1::uuid), "
            "       $2::bigint"
            "   ) ",
            Database::query_param_list
                (_check_handle)
                (_poll_msg_id)
        );
    }

    bool related_delete_domain_poll_message_exists(
        Fred::OperationContext& _ctx,
        const uuid&             _check_handle
    ) {
        return
            _ctx.get_conn().exec_params(
                "SELECT 1 "  // ...whatever, just to keep it smaller than "*"
                    "FROM contact_check_poll_message_map AS p_m_map "
                        "JOIN message AS m          ON p_m_map.poll_message_id = m.id "
                        "JOIN messagetype AS mtype  ON m.msgtype = mtype.id "
                        "JOIN contact_check AS c_ch ON p_m_map.contact_check_id = c_ch.id "
                    "WHERE "
                        "c_ch.handle = $1::uuid "
                        "AND mtype.name = $2::varchar "
                    "LIMIT 1 ", // going for bool value eventually, one would be enough
                Database::query_param_list
                    (_check_handle)
                    (Fred::Poll::DELETE_DOMAIN)
            ).size() > 0;
    }
}
