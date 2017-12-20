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

#ifndef NOTIFICATION_DATA_H_D4156D517F0D4FF2A749FC8234B90A27
#define NOTIFICATION_DATA_H_D4156D517F0D4FF2A749FC8234B90A27

#include <string>

namespace Epp {

struct NotificationData {
    const std::string client_transaction_handle;
    bool epp_notification_disabled;
    const std::string dont_notify_client_transaction_handles_with_this_prefix;

    NotificationData(
            const std::string& _client_transaction_handle,
            bool _epp_notification_disabled,
            const std::string& _dont_notify_client_transaction_handles_with_this_prefix)
        : client_transaction_handle(_client_transaction_handle),
          epp_notification_disabled(_epp_notification_disabled),
          dont_notify_client_transaction_handles_with_this_prefix(_dont_notify_client_transaction_handles_with_this_prefix)
    { }
};

} // namespace Epp

#endif
