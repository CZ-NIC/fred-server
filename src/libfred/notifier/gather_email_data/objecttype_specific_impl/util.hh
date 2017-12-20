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

/**
 *  @file
 */

#ifndef FREDLIB_NOTIFIER2_OBJECT_TYPE_UTIL_75445343456365
#define FREDLIB_NOTIFIER2_OBJECT_TYPE_UTIL_75445343456365

#include "src/libfred/opcontext.hh"
#include "src/libfred/notifier/event_on_object_enum.hh"
#include "src/libfred/notifier/exception.hh"

#include <boost/date_time/posix_time/ptime.hpp>

#include <string>

namespace Notification {

    inline boost::posix_time::ptime get_utc_time_of_event(
        LibFred::OperationContext& _ctx,
        notified_event _event,
        unsigned long long _last_history_id
    ) {
        if(
               _event != created
            && _event != updated
            && _event != transferred
            && _event != renewed
            && _event != deleted
        ) {
            throw ExceptionEventNotImplemented();
        }

        const std::string column_of_interest =
            _event == created || _event == updated || _event == transferred || _event == renewed
            ?   "valid_from"
            :   "valid_to";

        Database::query_param_list p;
        Database::Result time_res = _ctx.get_conn().exec_params(
            "SELECT " + column_of_interest + " AS the_time_ "
            "FROM history "
            "WHERE id = $" + p.add(_last_history_id) + "::INT ",
            p
        );

        if(time_res.size() != 1) {
            throw ExceptionUnknownHistoryId();
        }

        return
            boost::posix_time::time_from_string(
                static_cast<std::string>(
                    time_res[0]["the_time_"]
                )
            );
    }
}

#endif
