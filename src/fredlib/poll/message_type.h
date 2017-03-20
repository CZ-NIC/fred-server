/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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
 *  poll message types
 */

#ifndef MESSAGE_TYPE_H_B95C664FD50B40C2B5A521511970095C
#define MESSAGE_TYPE_H_B95C664FD50B40C2B5A521511970095C

#include "util/enum_conversion.h"

namespace Fred {
namespace Poll {

struct MessageType
{
    enum Enum
    {
        credit,
        request_fee_info,

        techcheck,

        transfer_contact,
        transfer_domain,
        transfer_nsset,
        transfer_keyset,

        idle_delete_contact,
        idle_delete_domain,
        idle_delete_nsset,
        idle_delete_keyset,

        imp_expiration,
        expiration,
        outzone,

        imp_validation,
        validation,

        update_domain,
        update_nsset,
        update_keyset,

        delete_contact,
        delete_domain
    };
};

}//namespace Fred::Poll
}//namespace Fred

namespace Conversion {
namespace Enums {

inline std::string to_db_handle(Fred::Poll::MessageType::Enum value)
{
    switch (value)
    {
        case Fred::Poll::MessageType::credit: return "credit";
        case Fred::Poll::MessageType::request_fee_info: return "request_fee_info";

        case Fred::Poll::MessageType::techcheck: return "techcheck";

        case Fred::Poll::MessageType::transfer_contact: return "transfer_contact";
        case Fred::Poll::MessageType::transfer_domain: return "transfer_domain";
        case Fred::Poll::MessageType::transfer_nsset: return "transfer_nsset";
        case Fred::Poll::MessageType::transfer_keyset: return "transfer_keyset";

        case Fred::Poll::MessageType::idle_delete_contact: return "idle_delete_contact";
        case Fred::Poll::MessageType::idle_delete_domain: return "idle_delete_domain";
        case Fred::Poll::MessageType::idle_delete_nsset: return "idle_delete_nsset";
        case Fred::Poll::MessageType::idle_delete_keyset: return "idle_delete_keyset";

        case Fred::Poll::MessageType::imp_expiration: return "imp_expiration";
        case Fred::Poll::MessageType::expiration: return "expiration";
        case Fred::Poll::MessageType::outzone: return "outzone";

        case Fred::Poll::MessageType::imp_validation: return "imp_validation";
        case Fred::Poll::MessageType::validation: return "validation";

        case Fred::Poll::MessageType::update_domain: return "update_domain";
        case Fred::Poll::MessageType::update_nsset: return "update_nsset";
        case Fred::Poll::MessageType::update_keyset: return "update_keyset";

        case Fred::Poll::MessageType::delete_contact: return "delete_contact";
        case Fred::Poll::MessageType::delete_domain: return "delete_domain";
    }
    throw std::invalid_argument("value doesn't exist in Fred::Poll::MessageType::Enum");
};

template < >
inline Fred::Poll::MessageType::Enum from_db_handle<Fred::Poll::MessageType>(const std::string &db_handle)
{
    if (to_db_handle(Fred::Poll::MessageType::credit) == db_handle) { return Fred::Poll::MessageType::credit; }
    if (to_db_handle(Fred::Poll::MessageType::request_fee_info) == db_handle) { return Fred::Poll::MessageType::request_fee_info; }

    if (to_db_handle(Fred::Poll::MessageType::techcheck) == db_handle) { return Fred::Poll::MessageType::techcheck; }

    if (to_db_handle(Fred::Poll::MessageType::transfer_contact) == db_handle) { return Fred::Poll::MessageType::transfer_contact; }
    if (to_db_handle(Fred::Poll::MessageType::transfer_domain) == db_handle) { return Fred::Poll::MessageType::transfer_domain; }
    if (to_db_handle(Fred::Poll::MessageType::transfer_nsset) == db_handle) { return Fred::Poll::MessageType::transfer_nsset; }
    if (to_db_handle(Fred::Poll::MessageType::transfer_keyset) == db_handle) { return Fred::Poll::MessageType::transfer_keyset; }

    if (to_db_handle(Fred::Poll::MessageType::idle_delete_contact) == db_handle) { return Fred::Poll::MessageType::idle_delete_contact; }
    if (to_db_handle(Fred::Poll::MessageType::idle_delete_domain) == db_handle) { return Fred::Poll::MessageType::idle_delete_domain; }
    if (to_db_handle(Fred::Poll::MessageType::idle_delete_nsset) == db_handle) { return Fred::Poll::MessageType::idle_delete_nsset; }
    if (to_db_handle(Fred::Poll::MessageType::idle_delete_keyset) == db_handle) { return Fred::Poll::MessageType::idle_delete_keyset; }

    if (to_db_handle(Fred::Poll::MessageType::imp_expiration) == db_handle) { return Fred::Poll::MessageType::imp_expiration; }
    if (to_db_handle(Fred::Poll::MessageType::expiration) == db_handle) { return Fred::Poll::MessageType::expiration; }
    if (to_db_handle(Fred::Poll::MessageType::outzone) == db_handle) { return Fred::Poll::MessageType::outzone; }

    if (to_db_handle(Fred::Poll::MessageType::imp_validation) == db_handle) { return Fred::Poll::MessageType::imp_validation; }
    if (to_db_handle(Fred::Poll::MessageType::validation) == db_handle) { return Fred::Poll::MessageType::validation; }

    if (to_db_handle(Fred::Poll::MessageType::update_domain) == db_handle) { return Fred::Poll::MessageType::update_domain; }
    if (to_db_handle(Fred::Poll::MessageType::update_nsset) == db_handle) { return Fred::Poll::MessageType::update_nsset; }
    if (to_db_handle(Fred::Poll::MessageType::update_keyset) == db_handle) { return Fred::Poll::MessageType::update_keyset; }

    if (to_db_handle(Fred::Poll::MessageType::delete_contact) == db_handle) { return Fred::Poll::MessageType::delete_contact; }
    if (to_db_handle(Fred::Poll::MessageType::delete_domain) == db_handle) { return Fred::Poll::MessageType::delete_domain; }

    throw std::invalid_argument("handle \"" + db_handle + "\" isn't convertible to Fred::Poll::MessageType::Enum");
}

}//namespace Conversion::Enums
}//namespace Conversion

#endif//MESSAGE_TYPE_H_B95C664FD50B40C2B5A521511970095C
