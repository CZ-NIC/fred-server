#include "src/admin/contact/verification/enqueue_check.h"
#include <fredlib/admin_contact_verification.h>

#include "src/fredlib/db_settings.h"

#include "util/log/context.h"

namespace  Admin {
    std::string enqueue_check(
        Fred::OperationContext&         _ctx,
        unsigned long long              _contact_id,
        const std::string&              _testsuite_handle,
        Optional<unsigned long long>    _logd_request_id
    ) {
        Logging::Context log("enqueue_check");

        try {
            std::string created_handle = Fred::CreateContactCheck(
                _contact_id,
                _testsuite_handle,
                _logd_request_id
            ).exec(_ctx);

            Fred::InfoContactCheckOutput info = Fred::InfoContactCheck(
                uuid::from_string(created_handle)
            ).exec(_ctx);

            Database::Result obsolete_handles_res = _ctx.get_conn().exec_params(
                "SELECT c_ch.handle as handle_ "
                "   FROM contact_check AS c_ch "
                "       JOIN enum_contact_check_status AS enum_status ON c_ch.enum_contact_check_status_id = enum_status.id "
                "       JOIN contact_history AS c_h1 ON c_ch.contact_history_id = c_h1.historyid "
                "       JOIN contact_history AS c_h2 ON c_h1.id = c_h2.id "
                "   WHERE enum_status.handle = $1::varchar "
                "       AND c_h2.historyid = $2::integer "
                "       AND c_ch.handle != $3::uuid ",
                Database::query_param_list
                    (Fred::ContactCheckStatus::ENQUEUED)
                    (info.contact_history_id)
                    (created_handle)
            );

            for(Database::Result::Iterator it = obsolete_handles_res.begin();
                it != obsolete_handles_res.end();
                ++it
            ) {
                Fred::UpdateContactCheck(
                    uuid::from_string( static_cast<std::string>( (*it)["handle_"]) ),
                    Fred::ContactCheckStatus::INVALIDATED,
                    _logd_request_id
                ).exec(_ctx);
            }

            return created_handle;
        } catch (const Fred::ExceptionUnknownContactId& ) {
            throw Admin::ExceptionUnknownContactId();
        } catch (const Fred::ExceptionUnknownTestsuiteHandle& ) {
            throw Admin::ExceptionUnknownTestsuiteHandle();
        }
    }
}
