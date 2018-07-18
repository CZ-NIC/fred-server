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
 *  end registrar group membership
 */

#ifndef END_REGISTRAR_GROUP_MEMBERSHIP_HH_6D432285E8894224BFF3680B7A8ED2A5
#define END_REGISTRAR_GROUP_MEMBERSHIP_HH_6D432285E8894224BFF3680B7A8ED2A5

#include "src/libfred/opcontext.hh"

namespace LibFred {
namespace Registrar {

class EndRegistrarGroupMembership
{
public:
    EndRegistrarGroupMembership(
        unsigned long long _registrar_id,
        unsigned long long _group_id)
    : registrar_id_(_registrar_id),
      group_id_(_group_id)
    {}

    void exec(OperationContext& _ctx);

private:
    unsigned long long registrar_id_;
    unsigned long long group_id_;

};

} // namespace Registrar
} // namespace LibFred

#endif

