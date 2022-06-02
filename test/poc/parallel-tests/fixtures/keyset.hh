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

#ifndef KEYSET_HH_75469E375B1756C225E912ED569B5A25//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define KEYSET_HH_75469E375B1756C225E912ED569B5A25

#include "test/poc/parallel-tests/fixtures/set_state.hh"

#include "libfred/registrable_object/keyset/create_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset_data.hh"

#include <string>
#include <utility>


namespace Test {

struct Keyset
{
    explicit Keyset(LibFred::OperationContext& ctx, LibFred::CreateKeyset create);
    template <typename ...Flags>
    explicit Keyset(LibFred::OperationContext& ctx, LibFred::CreateKeyset create, Flags ...state)
        : Keyset{ctx, std::move(create)}
    {
        set_state(ctx, *this, state...);
    }
    LibFred::InfoKeysetData data;
};

namespace Setter {

LibFred::CreateKeyset keyset(LibFred::CreateKeyset create, int index = 0);

}//namespace Test::Setter

}//namespace Test

#endif//KEYSET_HH_75469E375B1756C225E912ED569B5A25
