/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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

#ifndef NSSET_HH_C065711DA31809AC96840112E87416A0//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define NSSET_HH_C065711DA31809AC96840112E87416A0

#include "test/poc/parallel-tests/fixtures/set_state.hh"

#include "libfred/registrable_object/nsset/create_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset_data.hh"

#include <string>
#include <utility>


namespace Test {

struct Nsset
{
    explicit Nsset(LibFred::OperationContext& ctx, LibFred::CreateNsset create);
    template <typename ...Flags>
    explicit Nsset(LibFred::OperationContext& ctx, LibFred::CreateNsset create, Flags ...state)
        : Nsset{ctx, std::move(create)}
    {
        set_state(ctx, *this, state...);
    }
    LibFred::InfoNssetData data;
};

namespace Setter {

LibFred::CreateNsset nsset(LibFred::CreateNsset create, int index = 0);

}//namespace Test::Setter

}//namespace Test

#endif//NSSET_HH_C065711DA31809AC96840112E87416A0
