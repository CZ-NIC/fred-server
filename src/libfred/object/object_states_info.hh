/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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

#ifndef OBJECT_STATES_INFO_H_0AE168B27E2343E8A623660E996C99E0
#define OBJECT_STATES_INFO_H_0AE168B27E2343E8A623660E996C99E0

#include "src/libfred/object/object_state.hh"
#include "src/libfred/object_state/get_object_states.hh"

#include <set>

namespace LibFred {

class ObjectStatesInfo
{
public:
    typedef std::vector<ObjectStateData> ObjectStates;


    explicit ObjectStatesInfo(const ObjectStates& _object_states)
    {
        for (ObjectStates::const_iterator object_state = _object_states.begin(); object_state != _object_states.end(); ++object_state)
        {
            presents_.insert(Conversion::Enums::from_db_handle<Object_State>(object_state->state_name));
        }
    }


    bool presents(Object_State::Enum _state) const
    {
        return presents_.find(_state) != presents_.end();
    }


    bool absents(Object_State::Enum _state) const
    {
        return !this->presents(_state);
    }


private:
    typedef std::set<Object_State::Enum> SetOfStates;

    SetOfStates presents_;
};

} // namespace LibFred

#endif
