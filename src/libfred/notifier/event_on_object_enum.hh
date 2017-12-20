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
 *  notified events definitions and database handles correspondence
 */

#ifndef FREDLIB_NOTIFIER2_EVENT_ON_OBJECT_ENUM_4245348310351
#define FREDLIB_NOTIFIER2_EVENT_ON_OBJECT_ENUM_4245348310351

#include "src/libfred/object/registry_object_type.hh"

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

namespace Notification {

enum notified_event {
    created,
    updated,
    transferred,
    deleted,    /* unfortunately delete is a C++ keyword therefore using past tense for all */
    renewed
};

struct ExceptionUnknownNotifiedEvent {
    const char* what() const { return "unknown notified event"; }
};

/** @throws ExceptionUnknownNotifiedEvent */
inline std::string to_db_handle(notified_event _input) {
    switch(_input) {
        case created:       return "created";
        case updated:       return "updated";
        case transferred:   return "transferred";
        case deleted:       return "deleted";
        case renewed:       return "renewed";
    };

    throw ExceptionUnknownNotifiedEvent();
}

/** @throws ExceptionUnknownNotifiedEvent */
inline notified_event notified_event_from_db_handle(const std::string& _handle) {
    if(_handle == "created"     ) { return created;     }
    if(_handle == "updated"     ) { return updated;     }
    if(_handle == "transferred" ) { return transferred; }
    if(_handle == "deleted"     ) { return deleted;     }
    if(_handle == "renewed"     ) { return renewed;     }

    throw ExceptionUnknownNotifiedEvent();
}


struct ExceptionInvalidEventOnObject {
    const char* what() const { return "invalid event on object"; }
};

class EventOnObject {
    const LibFred::object_type type_;
    const notified_event event_;

    public:
        /** @throws ExceptionInvalidEventOnObject */
        EventOnObject(
            LibFred::object_type _type,
            notified_event _event
        ) :
            type_(_type),
            event_(_event)
        {
            if(
                event_ == renewed
                &&
                ( type_ == LibFred::contact || type_ == LibFred::keyset || type_ == LibFred::nsset )
            ){ throw ExceptionInvalidEventOnObject(); }
        }
        notified_event get_event() const { return event_; }
        LibFred::object_type get_type() const { return type_; }
};

}

#endif
