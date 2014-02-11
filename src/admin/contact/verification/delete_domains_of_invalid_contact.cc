#include "src/admin/contact/verification/delete_domains_of_invalid_contact.h"

#include "src/fredlib/contact/verification/info_check.h"
#include "src/fredlib/contact/verification/enum_testsuite_handle.h"
#include "src/fredlib/contact/verification/enum_check_status.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/domain/delete_domain.h"

#include <set>

namespace  Admin {

    static std::set<unsigned long long> get_owned_domains_locking(
        Fred::OperationContext& _ctx,
        unsigned long long _contact_id);

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

        std::set<unsigned long long> domain_ids_to_delete =
            get_owned_domains_locking (
                _ctx,
                Fred::HistoryInfoContactByHistoryid(check_info.contact_history_id)
                .exec(_ctx)
                    .info_contact_data.id
            );

        for(std::set<unsigned long long>::const_iterator it = domain_ids_to_delete.begin();
            it != domain_ids_to_delete.end();
            ++it
        ){
            Fred::DeleteDomainById(*it).exec(_ctx);
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
}
