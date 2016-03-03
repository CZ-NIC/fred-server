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

/**
*  @file
*  header of Fred::Object::StatesInfo class
*/
#ifndef STATES_INFO_H_8E754FC03E96F93ACCE180D1B5BA0C82//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define STATES_INFO_H_8E754FC03E96F93ACCE180D1B5BA0C82

#include "src/fredlib/object/object_state.h"
#include "src/fredlib/object_state/get_object_states.h"

#include <set>

/// Fred matters
namespace Fred {
/// Fred objects matters
namespace Object {

class StatesInfo
{
public:
    typedef std::vector< ObjectStateData > ObjectStates;
    explicit StatesInfo(const ObjectStates &_data)
    {
        for (ObjectStates::const_iterator ptr = _data.begin(); ptr != _data.end(); ++ptr) {
            presents_.insert(Conversion::Enums::from_db_handle< State >(ptr->state_name));
        }
    }
    bool presents(State::Enum _state)const
    {
        return presents_.find(_state) != presents_.end();
    }
    bool absents(State::Enum _state)const
    {
        return !this->presents(_state);
    }
private:
    typedef std::set< State::Enum > SetOfStates;
    SetOfStates presents_;
};

}//Fred::Object
}//Fred

#endif//STATES_INFO_H_8E754FC03E96F93ACCE180D1B5BA0C82
