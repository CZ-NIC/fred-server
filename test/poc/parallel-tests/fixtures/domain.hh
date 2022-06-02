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

#ifndef DOMAIN_HH_15E05EC1513BD71E26238BC378DF25EE//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define DOMAIN_HH_15E05EC1513BD71E26238BC378DF25EE

#include "test/poc/parallel-tests/fixtures/set_state.hh"

#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/info_domain_data.hh"

#include <string>
#include <utility>


namespace Test {

struct Domain
{
    explicit Domain(LibFred::OperationContext& ctx, LibFred::CreateDomain create);
    template <typename ...Flags>
    explicit Domain(LibFred::OperationContext& ctx, LibFred::CreateDomain create, Flags ...state)
        : Domain{ctx, std::move(create)}
    {
        set_state(ctx, *this, state...);
    }
    LibFred::InfoDomainData data;
};

namespace Setter {

LibFred::CreateDomain domain(LibFred::CreateDomain create, int index = 0);
LibFred::CreateDomain enum_domain(LibFred::CreateDomain create, int index = 0);

}//namespace Test::Setter

}//namespace Test

#endif//DOMAIN_HH_15E05EC1513BD71E26238BC378DF25EE
