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

#ifndef MESSAGE_TYPE_HH_69BDFA38C9AF48A3A46B259DB22EFD5E
#define MESSAGE_TYPE_HH_69BDFA38C9AF48A3A46B259DB22EFD5E

#include "src/util/enum_conversion.hh"

namespace LibFred {
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

        update_contact,
        update_domain,
        update_nsset,
        update_keyset,

        delete_contact,
        delete_domain
    };
};

} // namespace LibFred::Poll
} // namespace LibFred

namespace Conversion {
namespace Enums {

inline std::string to_db_handle(LibFred::Poll::MessageType::Enum value)
{
    switch (value)
    {
        case LibFred::Poll::MessageType::credit: return "credit";
        case LibFred::Poll::MessageType::request_fee_info: return "request_fee_info";

        case LibFred::Poll::MessageType::techcheck: return "techcheck";

        case LibFred::Poll::MessageType::transfer_contact: return "transfer_contact";
        case LibFred::Poll::MessageType::transfer_domain: return "transfer_domain";
        case LibFred::Poll::MessageType::transfer_nsset: return "transfer_nsset";
        case LibFred::Poll::MessageType::transfer_keyset: return "transfer_keyset";

        case LibFred::Poll::MessageType::idle_delete_contact: return "idle_delete_contact";
        case LibFred::Poll::MessageType::idle_delete_domain: return "idle_delete_domain";
        case LibFred::Poll::MessageType::idle_delete_nsset: return "idle_delete_nsset";
        case LibFred::Poll::MessageType::idle_delete_keyset: return "idle_delete_keyset";

        case LibFred::Poll::MessageType::imp_expiration: return "imp_expiration";
        case LibFred::Poll::MessageType::expiration: return "expiration";
        case LibFred::Poll::MessageType::outzone: return "outzone";

        case LibFred::Poll::MessageType::imp_validation: return "imp_validation";
        case LibFred::Poll::MessageType::validation: return "validation";

        case LibFred::Poll::MessageType::update_contact: return "update_contact";
        case LibFred::Poll::MessageType::update_domain: return "update_domain";
        case LibFred::Poll::MessageType::update_nsset: return "update_nsset";
        case LibFred::Poll::MessageType::update_keyset: return "update_keyset";

        case LibFred::Poll::MessageType::delete_contact: return "delete_contact";
        case LibFred::Poll::MessageType::delete_domain: return "delete_domain";
    }
    throw std::invalid_argument("value doesn't exist in LibFred::Poll::MessageType::Enum");
};

template < >
inline LibFred::Poll::MessageType::Enum from_db_handle<LibFred::Poll::MessageType>(const std::string &db_handle)
{
    static const LibFred::Poll::MessageType::Enum possible_results[] =
            {
                LibFred::Poll::MessageType::credit,
                LibFred::Poll::MessageType::request_fee_info,

                LibFred::Poll::MessageType::techcheck,

                LibFred::Poll::MessageType::transfer_contact,
                LibFred::Poll::MessageType::transfer_domain,
                LibFred::Poll::MessageType::transfer_nsset,
                LibFred::Poll::MessageType::transfer_keyset,

                LibFred::Poll::MessageType::idle_delete_contact,
                LibFred::Poll::MessageType::idle_delete_domain,
                LibFred::Poll::MessageType::idle_delete_nsset,
                LibFred::Poll::MessageType::idle_delete_keyset,

                LibFred::Poll::MessageType::imp_expiration,
                LibFred::Poll::MessageType::expiration,
                LibFred::Poll::MessageType::outzone,

                LibFred::Poll::MessageType::imp_validation,
                LibFred::Poll::MessageType::validation,

                LibFred::Poll::MessageType::update_contact,
                LibFred::Poll::MessageType::update_domain,
                LibFred::Poll::MessageType::update_nsset,
                LibFred::Poll::MessageType::update_keyset,

                LibFred::Poll::MessageType::delete_contact,
                LibFred::Poll::MessageType::delete_domain
            };
    return inverse_transformation(db_handle, possible_results, to_db_handle);
}

} // namespace Conversion::Enums
} // namespace Conversion

#endif
