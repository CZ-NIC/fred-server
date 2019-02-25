/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/epp/contact/impl/get_personal_id_union.hh"

namespace Epp {
namespace Contact {
namespace Impl {

namespace {

struct GetPersonalIdUnionFromContactIdent : boost::static_visitor<LibFred::PersonalIdUnion>
{
    LibFred::PersonalIdUnion operator()(const ContactIdentValueOf<ContactIdentType::Op>& src) const
    {
        return LibFred::PersonalIdUnion::get_OP(src.value);
    }
    LibFred::PersonalIdUnion operator()(const ContactIdentValueOf<ContactIdentType::Pass>& src) const
    {
        return LibFred::PersonalIdUnion::get_PASS(src.value);
    }
    LibFred::PersonalIdUnion operator()(const ContactIdentValueOf<ContactIdentType::Ico>& src) const
    {
        return LibFred::PersonalIdUnion::get_ICO(src.value);
    }
    LibFred::PersonalIdUnion operator()(const ContactIdentValueOf<ContactIdentType::Mpsv>& src) const
    {
        return LibFred::PersonalIdUnion::get_MPSV(src.value);
    }
    LibFred::PersonalIdUnion operator()(const ContactIdentValueOf<ContactIdentType::Birthday>& src) const
    {
        return LibFred::PersonalIdUnion::get_BIRTHDAY(src.value);
    }
};

}//namespace Epp::Contact::Impl::{anonymous}

LibFred::PersonalIdUnion get_personal_id_union(const ContactIdent& ident)
{
    return boost::apply_visitor(GetPersonalIdUnionFromContactIdent(), ident);
}

}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp
