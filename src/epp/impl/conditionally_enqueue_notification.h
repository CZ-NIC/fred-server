/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SRC_EPP_CONDITIONALLY_ENQUEUE_NOTIFICATION_4534374535
#define SRC_EPP_CONDITIONALLY_ENQUEUE_NOTIFICATION_4534374535

#include <string>

#include "src/fredlib/notifier/event_on_object_enum.h"

namespace Epp {

/**
 * Creates notification request in case registrar is not system registrar and _client_transaction_handle is not beginning with _client_transaction_handles_prefix_not_to_notify.
 */
void conditionally_enqueue_notification(
    Notification::notified_event _event,
    unsigned long long _object_history_id_post_change,
    unsigned long long _registrar_id,
    const std::string& _server_transaction_handle,
    const std::string& _client_transaction_handle,
    bool _epp_notification_disabled,
    const std::string& _client_transaction_handles_prefix_not_to_notify
) throw();

}

#endif
