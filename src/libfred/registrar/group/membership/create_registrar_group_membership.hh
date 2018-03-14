/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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
 *  create registrar group membership
 */

#ifndef CREATE_REGISTRAR_GROUP_MEMBERSHIP_HH_A101FCFEC90C4CB799AFBE8F53F22EF6
#define CREATE_REGISTRAR_GROUP_MEMBERSHIP_HH_A101FCFEC90C4CB799AFBE8F53F22EF6

#include "src/libfred/opcontext.hh"
#include "src/util/optional_value.hh"

#include <boost/date_time/gregorian/gregorian.hpp>

namespace LibFred {
namespace Registrar {

class CreateRegistrarGroupMembership
{
public:
    CreateRegistrarGroupMembership(
        unsigned long long _registrar_id,
        unsigned long long _group_id,
        boost::gregorian::date _member_from,
        boost::gregorian::date _member_until = {})
    : registrar_id_(_registrar_id),
      group_id_(_group_id),
      member_from_(_member_from),
      member_until_(_member_until)
    {}

    unsigned long long exec(OperationContext& _ctx);

private:
    unsigned long long registrar_id_;
    unsigned long long group_id_;
    boost::gregorian::date member_from_;
    boost::gregorian::date member_until_;

};

} // namespace Registrar
} // namespace LibFred

#endif
