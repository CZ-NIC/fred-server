/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/epp/impl/conditionally_enqueue_notification.hh"

#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/notifier/enqueue_notification.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrar/info_registrar.hh"

namespace Epp {

void conditionally_enqueue_notification(
        const Notification::notified_event _event,
        const unsigned long long _object_history_id_post_change,
        const SessionData& _session_data,
        const NotificationData& _notification_data)
{

    try {
        LibFred::OperationContextCreator ctx;

        try {
            if (!_notification_data.epp_notification_disabled)
            {
                const bool client_transaction_handle_starts_with_given_prefix = boost::starts_with(
                        _notification_data.client_transaction_handle,
                        _notification_data.dont_notify_client_transaction_handles_with_this_prefix);

                const LibFred::InfoRegistrarData session_registrar =
                    LibFred::InfoRegistrarById(_session_data.registrar_id).exec(ctx).info_registrar_data;
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
