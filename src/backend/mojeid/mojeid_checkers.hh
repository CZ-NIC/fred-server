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

#ifndef MOJEID_CHECKERS_HH_717A66E63EE24C88BEE864D37E955CBA
#define MOJEID_CHECKERS_HH_717A66E63EE24C88BEE864D37E955CBA

#include "src/backend/mojeid/checkers.hh"
#include "src/libfred/registrable_object/contact/info_contact_data.hh"
#include "src/libfred/object/object_states_info.hh"

namespace LibFred {
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

#endif