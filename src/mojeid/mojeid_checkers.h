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
 *  header of mojeid checkers
 */

#ifndef MOJEID2_CHECKERS_H_3F3071E19E8ADD1CB7438F97BBE37530//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define MOJEID2_CHECKERS_H_3F3071E19E8ADD1CB7438F97BBE37530

#include "src/mojeid/checkers.h"
#include "src/fredlib/contact/info_contact_data.h"
#include "src/fredlib/object/object_states_info.h"

namespace Fred {
namespace MojeID {
namespace Check {

struct states_before_transfer_into_mojeid
{
    states_before_transfer_into_mojeid(const ObjectStatesInfo &_states_info)
    :   server_user_blocked   (_states_info.presents(Object_State::server_transfer_prohibited) ||
                               _states_info.presents(Object_State::server_update_prohibited) ||
                               _states_info.presents(Object_State::server_delete_prohibited)),
        server_admin_blocked  (_states_info.presents(Object_State::server_blocked)),
        mojeid_contact_present(_states_info.presents(Object_State::mojeid_contact))
    { }
    bool success()const
    {
        return !(server_user_blocked ||
                 server_admin_blocked ||
                 mojeid_contact_present);
    }
    bool server_user_blocked:1;
    bool server_admin_blocked:1;
    bool mojeid_contact_present:1;
};

}//Fred::MojeID::Check
}//Fred::MojeID
}//Fred

#endif//MOJEID2_CHECKERS_H_3F3071E19E8ADD1CB7438F97BBE37530
