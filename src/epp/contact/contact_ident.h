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

#ifndef CONTACT_IDENT_H_8A731CBF05BB43A05B7FE62544069231//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CONTACT_IDENT_H_8A731CBF05BB43A05B7FE62544069231

#include <string>
#include <boost/variant.hpp>

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

typedef boost::variant<
        ContactIdentValueOf<ContactIdentType::Op>,
        ContactIdentValueOf<ContactIdentType::Pass>,
        ContactIdentValueOf<ContactIdentType::Ico>,
        ContactIdentValueOf<ContactIdentType::Mpsv>,
        ContactIdentValueOf<ContactIdentType::Birthday> > ContactIdent;

}//namespace Epp::Contact
}//namespace Epp

#endif//CONTACT_IDENT_H_8A731CBF05BB43A05B7FE62544069231
