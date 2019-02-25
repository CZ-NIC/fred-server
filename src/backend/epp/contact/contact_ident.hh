/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CONTACT_IDENT_HH_A5B660D28E014495A5CC02AF4B809CC9
#define CONTACT_IDENT_HH_A5B660D28E014495A5CC02AF4B809CC9

#include <boost/variant.hpp>

#include <string>

namespace Epp {
namespace Contact {

struct ContactIdentType
{
    struct Op;
    struct Pass;
    struct Ico;
    struct Mpsv;
    struct Birthday;
};

template <typename T>
struct ContactIdentValueOf
{
    explicit ContactIdentValueOf(const std::string& src):value(src) { }
    std::string value;
};

template <typename ...T>
using ContactIdentOf = boost::variant<ContactIdentValueOf<T>...>;

using ContactIdent = ContactIdentOf<
        ContactIdentType::Op,
        ContactIdentType::Pass,
        ContactIdentType::Ico,
        ContactIdentType::Mpsv,
        ContactIdentType::Birthday>;

template <typename T>
ContactIdent make_contact_ident(const std::string& src)
{
    return ContactIdent(ContactIdentValueOf<T>(src));
}

}//namespace Epp::Contact
}//namespace Epp

#endif
