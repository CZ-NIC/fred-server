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
 *  create registrar group
 */

#ifndef CREATE_REGISTRAR_GROUP_HH_C0ABF18E040A4B2EAD047E8E96986311
#define CREATE_REGISTRAR_GROUP_HH_C0ABF18E040A4B2EAD047E8E96986311

#include "src/libfred/opcontext.hh"

#include <string>

namespace LibFred {
namespace Registrar {

class CreateRegistrarGroup
{
public:
    explicit CreateRegistrarGroup(const std::string& _group_name)
    : group_name_(_group_name)
    {}

    unsigned long long exec(OperationContext& _ctx);

private:
    const std::string group_name_;

};

} // namespace LibFred
} // namespace Registrar

#endif
