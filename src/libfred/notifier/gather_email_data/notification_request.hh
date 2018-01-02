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

#ifndef NOTIFICATION_REQUEST_HH_BD8E134CD2B9406FA369AA6336BB6A0E
#define NOTIFICATION_REQUEST_HH_BD8E134CD2B9406FA369AA6336BB6A0E

#include "src/libfred/notifier/event_on_object_enum.hh"

#include <string>

namespace Notification {

/**
 * immutable data
 * in case you realy need to mutate data - use the constructor (or think about bigger rewriting as it was not intended)
 */
struct notification_request {
    const EventOnObject       event;
    const unsigned long long  done_by_registrar;
    const unsigned long long  history_id_post_change;
    const std::string         svtrid;

    notification_request(
        const EventOnObject&    _event,
        unsigned long long      _done_by_registrar,
        unsigned long long      _history_id_post_change,
        const std::string&      _svtrid
    ) :
        event(_event),
        done_by_registrar(_done_by_registrar),
        history_id_post_change(_history_id_post_change),
        svtrid(_svtrid)
    { }
};

}
#endif
