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

#ifndef MESSAGE_TYPE_HH_1D39988A7F594B28BB40341AE7DB517B
#define MESSAGE_TYPE_HH_1D39988A7F594B28BB40341AE7DB517B

#include "util/enum_conversion.hh"

namespace Epp {
namespace Poll {

struct MessageType
{
    enum Enum
    {
        credit,
        techcheck,
        transfer_contact,
        transfer_nsset,
        transfer_domain,
        idle_delete_contact,
        idle_delete_nsset,
        idle_delete_domain,
        imp_expiration,
        expiration,
        imp_validation,
        validation,
        outzone,
        transfer_keyset,
        idle_delete_keyset,
        request_fee_info,
        update_contact,
        update_domain,
        update_nsset,
        update_keyset,
        delete_contact,
        delete_domain
    };
};

} // namespace Epp::Poll
} // namespace Epp

namespace Conversion {
namespace Enums {

inline std::string to_db_handle(Epp::Poll::MessageType::Enum value)
{
    switch (value)
    {
        case Epp::Poll::MessageType::credit: return "credit";
        case Epp::Poll::MessageType::techcheck: return "techcheck";
        case Epp::Poll::MessageType::transfer_contact: return "transfer_contact";
        case Epp::Poll::MessageType::transfer_nsset: return "transfer_nsset";
        case Epp::Poll::MessageType::transfer_domain: return "transfer_domain";
        case Epp::Poll::MessageType::idle_delete_contact: return "idle_delete_contact";
        case Epp::Poll::MessageType::idle_delete_nsset: return "idle_delete_nsset";
        case Epp::Poll::MessageType::idle_delete_domain: return "idle_delete_domain";
        case Epp::Poll::MessageType::imp_expiration: return "imp_expiration";
        case Epp::Poll::MessageType::expiration: return "expiration";
        case Epp::Poll::MessageType::imp_validation: return "imp_validation";
        case Epp::Poll::MessageType::validation: return "validation";
        case Epp::Poll::MessageType::outzone: return "outzone";
        case Epp::Poll::MessageType::transfer_keyset: return "transfer_keyset";
        case Epp::Poll::MessageType::idle_delete_keyset: return "idle_delete_keyset";
        case Epp::Poll::MessageType::request_fee_info: return "request_fee_info";
        case Epp::Poll::MessageType::update_contact: return "update_contact";
        case Epp::Poll::MessageType::update_domain: return "update_domain";
        case Epp::Poll::MessageType::update_nsset: return "update_nsset";
        case Epp::Poll::MessageType::update_keyset: return "update_keyset";
        case Epp::Poll::MessageType::delete_contact: return "delete_contact";
        case Epp::Poll::MessageType::delete_domain: return "delete_domain";
    }
    throw std::invalid_argument("value does not exist in Epp::Poll::MessageType::Enum");
}

template<>
inline Epp::Poll::MessageType::Enum from_db_handle<Epp::Poll::MessageType>(const std::string& db_handle)
{
    if (to_db_handle(Epp::Poll::MessageType::credit) == db_handle) {
        return Epp::Poll::MessageType::credit;
    }
    if (to_db_handle(Epp::Poll::MessageType::techcheck) == db_handle) {
        return Epp::Poll::MessageType::techcheck;
    }
    if (to_db_handle(Epp::Poll::MessageType::transfer_contact) == db_handle) {
        return Epp::Poll::MessageType::transfer_contact;
    }
    if (to_db_handle(Epp::Poll::MessageType::transfer_nsset) == db_handle) {
        return Epp::Poll::MessageType::transfer_nsset;
    }
    if (to_db_handle(Epp::Poll::MessageType::transfer_domain) == db_handle) {
        return Epp::Poll::MessageType::transfer_domain;
    }
    if (to_db_handle(Epp::Poll::MessageType::idle_delete_contact) == db_handle) {
        return Epp::Poll::MessageType::idle_delete_contact;
    }
    if (to_db_handle(Epp::Poll::MessageType::idle_delete_nsset) == db_handle) {
        return Epp::Poll::MessageType::idle_delete_nsset;
    }
    if (to_db_handle(Epp::Poll::MessageType::idle_delete_domain) == db_handle) {
        return Epp::Poll::MessageType::idle_delete_domain;
    }
    if (to_db_handle(Epp::Poll::MessageType::imp_expiration) == db_handle) {
        return Epp::Poll::MessageType::imp_expiration;
    }
    if (to_db_handle(Epp::Poll::MessageType::expiration) == db_handle) {
        return Epp::Poll::MessageType::expiration;
    }
    if (to_db_handle(Epp::Poll::MessageType::imp_validation) == db_handle) {
        return Epp::Poll::MessageType::imp_validation;
    }
    if (to_db_handle(Epp::Poll::MessageType::validation) == db_handle) {
        return Epp::Poll::MessageType::validation;
    }
    if (to_db_handle(Epp::Poll::MessageType::outzone) == db_handle) {
        return Epp::Poll::MessageType::outzone;
    }
    if (to_db_handle(Epp::Poll::MessageType::transfer_keyset) == db_handle) {
        return Epp::Poll::MessageType::transfer_keyset;
    }
    if (to_db_handle(Epp::Poll::MessageType::idle_delete_keyset) == db_handle) {
        return Epp::Poll::MessageType::idle_delete_keyset;
    }
    if (to_db_handle(Epp::Poll::MessageType::request_fee_info) == db_handle) {
        return Epp::Poll::MessageType::request_fee_info;
    }
    if (to_db_handle(Epp::Poll::MessageType::update_contact) == db_handle) {
        return Epp::Poll::MessageType::update_contact;
    }
    if (to_db_handle(Epp::Poll::MessageType::update_domain) == db_handle) {
        return Epp::Poll::MessageType::update_domain;
    }
    if (to_db_handle(Epp::Poll::MessageType::update_nsset) == db_handle) {
        return Epp::Poll::MessageType::update_nsset;
    }
    if (to_db_handle(Epp::Poll::MessageType::update_keyset) == db_handle) {
        return Epp::Poll::MessageType::update_keyset;
    }
    if (to_db_handle(Epp::Poll::MessageType::delete_contact) == db_handle) {
        return Epp::Poll::MessageType::delete_contact;
    }
    if (to_db_handle(Epp::Poll::MessageType::delete_domain) == db_handle) {
        return Epp::Poll::MessageType::delete_domain;
    }
    throw std::invalid_argument("handle \"" + db_handle + "\" is not convertible to Epp::Poll::MessageType::Enum");
}

} // namespace Conversion::Enums
} // namespace Conversion

#endif
