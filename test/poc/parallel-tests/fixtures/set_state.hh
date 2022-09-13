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

#ifndef SET_STATE_HH_CDC42D9754B0D8AE8A0A48BDF4B16F7B//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define SET_STATE_HH_CDC42D9754B0D8AE8A0A48BDF4B16F7B

#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/registrable_object/state.hh"

namespace Test {

template <typename ...Flags> struct MakeStatusList;

template <LibFred::RegistrableObject::State::Flag::Visibility visibility, const char* name>
using ManualStateFlag = LibFred::RegistrableObject::State::Flag::Type<LibFred::RegistrableObject::State::Flag::Manipulation::manual, visibility, name>;

template <LibFred::RegistrableObject::State::Flag::Visibility visibility, const char* name>
struct MakeStatusList<ManualStateFlag<visibility, name>>
{
    MakeStatusList()
        : status_list{}
    {
        insert_into(status_list);
    }
    static void insert_into(LibFred::StatusList& status_list)
    {
        status_list.insert(name);
    }
    LibFred::StatusList status_list;
};

template <LibFred::RegistrableObject::State::Flag::Visibility visibility, const char* name, typename ...Flags>
struct MakeStatusList<ManualStateFlag<visibility, name>, Flags...>
{
    MakeStatusList()
        : status_list{}
    {
        insert_into(status_list);
    }
    static void insert_into(LibFred::StatusList& status_list)
    {
        status_list.insert(name);
        MakeStatusList<Flags...>::insert_into(status_list);
    }
    LibFred::StatusList status_list;
};

template <typename Object, typename ...Flags>
void set_state(::LibFred::OperationContext& ctx, const Object& object, Flags...)
{
    LibFred::CreateObjectStateRequestId{object.data.id, MakeStatusList<Flags...>{}.status_list}.exec(ctx);
    LibFred::PerformObjectStateRequest{object.data.id}.exec(ctx);
}

}//namespace Test

#endif//SET_STATE_HH_CDC42D9754B0D8AE8A0A48BDF4B16F7B
