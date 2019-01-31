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

#ifndef UTIL_HH_20EA4D52B76F4169A2CC6CB531BDF305
#define UTIL_HH_20EA4D52B76F4169A2CC6CB531BDF305

#include "libfred/opcontext.hh"
#include "test/setup/fixtures.hh"

namespace Test {
namespace LibFred {
namespace Contact {


struct autocommitting_context
    : virtual Test::instantiate_db_template
{
    ::LibFred::OperationContextCreator ctx;


    virtual ~autocommitting_context()
    {
        ctx.commit_transaction();
    }


};

struct autorollbacking_context
    : virtual Test::instantiate_db_template
{
    ::LibFred::OperationContextCreator ctx;

    void commit_transaction()
    {
        ctx.commit_transaction();
    }

};


} // namespace Test::LibFred::Contact
} // namespace Test::LibFred
} // namespace Test

#endif
