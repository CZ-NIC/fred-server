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

#ifndef CONTACT_HH_BC8B4F00A2917D110FD9CEDA02DD8093//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define CONTACT_HH_BC8B4F00A2917D110FD9CEDA02DD8093

#include "test/poc/parallel-tests/fixtures/set_state.hh"

#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/info_contact_data.hh"

#include <string>
#include <utility>


namespace Test {

struct Contact
{
    explicit Contact(LibFred::OperationContext& ctx, LibFred::CreateContact create);
    LibFred::InfoContactData data;
    template <typename ...Flags>
    explicit Contact(LibFred::OperationContext& ctx, LibFred::CreateContact create, Flags ...state)
        : Contact{ctx, std::move(create)}
    {
        set_state(ctx, *this, state...);
    }
};

namespace Setter {

LibFred::CreateContact contact(LibFred::CreateContact create, int index = 0);
LibFred::CreateContact company(LibFred::CreateContact create, int index = 0);

}//namespace Test::Setter

}//namespace Test

#endif//CONTACT_HH_BC8B4F00A2917D110FD9CEDA02DD8093
