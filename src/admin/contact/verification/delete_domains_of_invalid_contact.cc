#include "src/admin/contact/verification/delete_domains_of_invalid_contact.h"

#include "src/fredlib/contact/verification/info_check.h"
#include "src/fredlib/contact/verification/enum_testsuite_handle.h"
#include "src/fredlib/contact/verification/enum_check_status.h"

#include <fredlib/domain.h>
#include <fredlib/contact.h>
#include "src/fredlib/poll/create_delete_domain_poll_message.h"

#include <set>

namespace  Admin {

    static std::set<unsigned long long> get_owned_domains_locking(
        Fred::OperationContext& _ctx,
        unsigned long long _contact_id);

    static void store_check_poll_message_relation(
        Fred::OperationContext& _ctx,
        const std::string&      _check_handle,
        unsigned long long      _poll_msg_id);

    void delete_domains_of_invalid_contact(
        Fred::OperationContext& _ctx,
        const std::string&      _check_handle
    ) {
        Fred::InfoContactCheckOutput check_info = Fred::InfoContactCheck(_check_handle).exec(_ctx);

        if( check_info.testsuite_handle != Fred::TestsuiteHandle::MANUAL
            ||
            check_info.check_state_history.rbegin()->status_handle != Fred::ContactCheckStatus::FAIL
        ) {
            return;
        }

        Fred::InfoContactOutput contact_info = Fred::HistoryInfoContactByHistoryid(check_info.contact_history_id).exec(_ctx);

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
        const std::string&      _check_handle,
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
}
