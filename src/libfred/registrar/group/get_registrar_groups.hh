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
 *  get registrar groups
 */

#ifndef GET_REGISTRAR_GROUPS_HH_46B31EA66921472A91C8206114D86775
#define GET_REGISTRAR_GROUPS_HH_46B31EA66921472A91C8206114D86775

#include "src/libfred/opcontext.hh"
#include "src/libfred/registrar/group/registrar_group_type.hh"

namespace LibFred {
namespace Registrar {

class GetRegistrarGroups
{
public:
    std::vector<RegistrarGroup> exec(OperationContext& _ctx);
};

} // namespace Registrar
} // namespace LibFred

#endif
