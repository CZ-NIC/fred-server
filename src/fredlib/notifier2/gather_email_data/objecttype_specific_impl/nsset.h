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
 *  nsset specific notification implementation
 */

#ifndef FREDLIB_NOTIFIER2_OBJECT_TYPE_NSSET_397411054413
#define FREDLIB_NOTIFIER2_OBJECT_TYPE_NSSET_397411054413

#include "src/fredlib/opcontext.h"
#include "src/fredlib/notifier2/event_on_object_enum.h"

#include <string>
#include <map>

namespace Notification {

    std::map<std::string, std::string> gather_nsset_data_change(
        Fred::OperationContext& _ctx,
        const notified_event& _event,
        unsigned long long _history_id_post_change
    );

    std::set<unsigned long long> gather_contact_ids_to_notify_nsset_event(
        Fred::OperationContext& _ctx,
        notified_event _event,
        unsigned long long _history_id_after_change
    );
}

#endif
