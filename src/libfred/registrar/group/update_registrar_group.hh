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
 *  update registrar group
 */

#ifndef UPDATE_REGISTRAR_GROUP_HH_AC7B3AE57698429597EAE7BD1F3E66D2
#define UPDATE_REGISTRAR_GROUP_HH_AC7B3AE57698429597EAE7BD1F3E66D2

#include "src/libfred/opcontext.hh"

#include <string>

namespace LibFred {
namespace Registrar {

class UpdateRegistrarGroup
{
public:
    UpdateRegistrarGroup(unsigned long long _group_id,
                         const std::string& _group_name)
    : group_id_(_group_id),
      group_name_(_group_name)
    {}

    void exec(OperationContext& _ctx);

private:
    const unsigned long long group_id_;
    const std::string group_name_;

};

} // namespace Registrar
} // namespace LibFred

#endif
