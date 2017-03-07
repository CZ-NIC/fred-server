#include "src/epp/conditionally_enqueue_notification.h"

#include "src/epp/notification_data.h"
#include "src/epp/session_data.h"
#include "src/fredlib/notifier/enqueue_notification.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/registrar/info_registrar.h"

namespace Epp {

void conditionally_enqueue_notification(
        const Notification::notified_event _event,
        const unsigned long long _object_history_id_post_change,
        const SessionData& _session_data,
        const NotificationData& _notification_data)
{

    try {
        Fred::OperationContextCreator ctx;

        try {
            if (!_notification_data.epp_notification_disabled)
            {
                const bool client_transaction_handle_starts_with_given_prefix = boost::starts_with(
                        _notification_data.client_transaction_handle,
                        _notification_data.dont_notify_client_transaction_handles_with_this_prefix);

                const Fred::InfoRegistrarData session_registrar =
                    Fred::InfoRegistrarById(_session_data.registrar_id).exec(ctx).info_registrar_data;
                const bool is_system_registrar = session_registrar.system.get_value_or(false);

                const bool do_not_notify = client_transaction_handle_starts_with_given_prefix && is_system_registrar;

                if (do_not_notify)
                {
                    ctx.get_log().info(
                    "command notification avoided because of system registrar and cltrid prefix("
                    "registrar=" + boost::lexical_cast<std::string>(_session_data.registrar_id)
                    + " event="+ to_db_handle(_event)
                    + " object_historyid_post_change=" + boost::lexical_cast<std::string>(_object_history_id_post_change)
                    + " cltrid=" + _notification_data.client_transaction_handle
                    + " svtrid=" + _session_data.server_transaction_handle
                    +")");
                }
                else
                {
                    Notification::enqueue_notification(
                        ctx,
                        _event,
                        _session_data.registrar_id,
                        _object_history_id_post_change,
                        _session_data.server_transaction_handle
                    );

                    ctx.commit_transaction();
                }
            }

        } catch(...) {
            ctx.get_log().error(
                "Notification::enqueue_notification() failed. "
                "params { "
                    "_event="+ to_db_handle(_event) + ", "
                    "_done_by_registrar=" + boost::lexical_cast<std::string>(_session_data.registrar_id) + ", "
                    "_object_historyid_post_change=" + boost::lexical_cast<std::string>(_object_history_id_post_change) + ", "
                    "_svtrid=" + _session_data.server_transaction_handle + " "
                "}"
            );

            // not propagating exceptions further
        }

    } catch (...) {

        // suppress exceptions from catch section above

    }
}

}
