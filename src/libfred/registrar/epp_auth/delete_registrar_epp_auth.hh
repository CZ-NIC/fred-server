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

#ifndef DELETE_REGISTRAR_EPP_AUTH_HH_7FB5773EAB814D37A611447D8A29E4D8
#define DELETE_REGISTRAR_EPP_AUTH_HH_7FB5773EAB814D37A611447D8A29E4D8

#include "src/libfred/opcontext.hh"

namespace LibFred {
namespace Registrar {
namespace EppAuth {

class DeleteRegistrarEppAuth
{
public:
    explicit DeleteRegistrarEppAuth(unsigned long long _id);

    void exec(OperationContext& _ctx) const;

private:
    unsigned long long id_;
};

} // namespace LibFred::Registrar::EppAuth
} // namespace LibFred::Registrar
} // namespace LibFred

#endif
