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
#ifndef CONDITIONALLY_ENQUEUE_NOTIFICATION_HH_25CAE2C1511C47D88A707C8B09025B24
#define CONDITIONALLY_ENQUEUE_NOTIFICATION_HH_25CAE2C1511C47D88A707C8B09025B24

#include <string>

#include "src/backend/epp/notification_data.hh"
#include "src/backend/epp/session_data.hh"
#include "libfred/notifier/event_on_object_enum.hh"

namespace Epp {

/**
 * Creates notification request in case _session_data.registrar_id is not system registrar or
 * _notification_data.client_transaction_handle is not beginning with
 * _notification_data.dont_notify_client_transaction_handles_with_this_prefix
 */
void conditionally_enqueue_notification(
        Notification::notified_event _event,
        unsigned long long _object_history_id_post_change,
        const SessionData& _session_data,
        const NotificationData& _notification_data);

}

#endif
