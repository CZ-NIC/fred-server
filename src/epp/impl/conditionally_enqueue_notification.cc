#include "src/epp/impl/conditionally_enqueue_notification.h"

#include "src/fredlib/opcontext.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/notifier/enqueue_notification.h"

namespace Epp {

void conditionally_enqueue_notification(
    const Notification::notified_event _event,
    const unsigned long long _object_history_id_post_change,
    const unsigned long long _registrar_id,
    const std::string& _server_transaction_handle,
    const std::string& _client_transaction_handle,
    const bool _epp_notification_disabled,
    const std::string& _dont_notify_client_transaction_handles_with_this_prefix
) throw() {

    try {
        Fred::OperationContextCreator ctx;
        try {

            if (!_epp_notification_disabled)
            {
                if (_client_transaction_handle.substr( 0, _dont_notify_client_transaction_handles_with_this_prefix.length() )
                    == _dont_notify_client_transaction_handles_with_this_prefix
                    &&
                    Fred::InfoRegistrarById(_registrar_id).exec(ctx)
                        .info_registrar_data.system.get_value_or_default() // if Null given default is false ...
                )
                {
                    ctx.get_log().info(
                    "command notification avoided because of system registrar and cltrid prefix("
                    "registrar=" + boost::lexical_cast<std::string>(_registrar_id)
                    + " event="+ to_db_handle(_event)
                    + " object_historyid_post_change=" + boost::lexical_cast<std::string>(_object_history_id_post_change)
                    + " cltrid=" +_client_transaction_handle
                    + " svtrid=" + _server_transaction_handle
                    +")");
                }
                else
                {
                    Notification::enqueue_notification(
                        ctx,
                        _event,
                        _registrar_id,
                        _object_history_id_post_change,
                        _server_transaction_handle
                    );

                    ctx.commit_transaction();
                }
            }

        } catch(...) {
            ctx.get_log().error(
                "Notification::enqueue_notification() failed. "
                "params { "
                    "_event="+ to_db_handle(_event) + ", "
                    "_done_by_registrar=" + boost::lexical_cast<std::string>(_registrar_id) + ", "
                    "_object_historyid_post_change=" + boost::lexical_cast<std::string>(_object_history_id_post_change) + ", "
                    "_svtrid=" + _server_transaction_handle + " "
                "}"
            );

            /* not propagating exception further */
        }
    } catch (...) {
        /* second line of defense - needed because I would like to log exceptions if possible and that could throw */
    }
}

}
