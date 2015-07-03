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
*  header of Fred::Object::State class
*/
#ifndef GET_STATES_PRESENCE_H_8E754FC03E96F93ACCE180D1B5BA0C82//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define GET_STATES_PRESENCE_H_8E754FC03E96F93ACCE180D1B5BA0C82

#include "src/fredlib/object/object_state.h"
#include "src/fredlib/object/object_type.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"

/// Fred matters
namespace Fred {
/// Fred objects matters
namespace Object {

/**
 * @section sample Sample of intended usage
 * @code{.cpp}
 * typedef State::set< State::DELETE_CANDIDATE,
 *                     State::DELETE_WARNING,
 *                     State::VALIDATED_CONTACT > states_for_check;
 * Get< Type::CONTACT >(contact_id).states< states_for_check >().presence().exec(ctx);
 * @endcode
 */

template < Type::Value OBJECT_TYPE >
class Get
{
public:
    Get(Id _object_id);
    template < typename STATES >
    class States
    {
    public:
        class Presence
        {
            struct Result;
            Result exec(OperationContext &_ctx)const;
        };
        Presence presence()const;
    };
    template < typename STATES >
    States< STATES > states()const;
private:
    const Id object_id_;
};

}//Fred::Object
}//Fred

#endif//GET_STATES_PRESENCE_H_8E754FC03E96F93ACCE180D1B5BA0C82
