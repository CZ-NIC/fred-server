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
 *  registrar group membership types
 */

#ifndef REGISTRAR_GROUP_MEMBERSHIP_TYPES_HH_D615399AE64341FB8A358AA941296CEB
#define REGISTRAR_GROUP_MEMBERSHIP_TYPES_HH_D615399AE64341FB8A358AA941296CEB

#include <boost/date_time/gregorian/gregorian.hpp>

namespace LibFred {
namespace Registrar {

struct GroupMembershipByGroup
{
    unsigned long long membership_id;
    unsigned long long registrar_id;
    boost::gregorian::date member_from;
    boost::gregorian::date member_until;
};

struct GroupMembershipByRegistrar
{
    unsigned long long membership_id;
    unsigned long long group_id;
    boost::gregorian::date member_from;
    boost::gregorian::date member_until;
};

} // namespace Registrar
} // namespace LibFred

#endif
