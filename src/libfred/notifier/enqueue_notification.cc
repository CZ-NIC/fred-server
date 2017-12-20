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
 */

#include "src/libfred/notifier/enqueue_notification.hh"

#include "src/util/db/query_param.hh"

namespace Notification {

void enqueue_notification(
    LibFred::OperationContext& _ctx,
    const notified_event    _event,
    unsigned long long      _done_by_registrar,
    unsigned long long      _object_historyid_post_change,
    const std::string&      _svtrid
) {
    Database::query_param_list p;
    _ctx.get_conn().exec_params(
        "INSERT INTO notification_queue "
        "("
            "change, "
            "done_by_registrar, "
            "historyid_post_change, "
            "svtrid "
        ") "
        "VALUES( "
            "$" + p.add(to_db_handle(_event)) + "::notified_event, "
            "$" + p.add(_done_by_registrar) + "::integer, "
            "$" + p.add(_object_historyid_post_change) + "::integer, "
            "$" + p.add(_svtrid) + "::text "
        ")",
        p
    );
}

}
