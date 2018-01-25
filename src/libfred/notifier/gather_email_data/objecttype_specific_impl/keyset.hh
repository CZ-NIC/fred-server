/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

/**
 *  @file
 *  keyset specific notification implementation
 */

#ifndef KEYSET_HH_DE68A983920F46AB9F99A43E9EE4CB50
#define KEYSET_HH_DE68A983920F46AB9F99A43E9EE4CB50

#include "src/libfred/opcontext.hh"
#include "src/libfred/notifier/event_on_object_enum.hh"

#include <string>
#include <map>

namespace Notification {

    std::map<std::string, std::string> gather_keyset_data_change(
        LibFred::OperationContext& _ctx,
        const notified_event& _event,
        unsigned long long _history_id_post_change
    );

    std::set<unsigned long long> gather_contact_ids_to_notify_keyset_event(
        LibFred::OperationContext& _ctx,
        notified_event _event,
        unsigned long long _history_id_after_change
    );
}

#endif