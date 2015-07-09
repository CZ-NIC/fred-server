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

#include "src/mojeid/mojeid2_checkers.h"
#include "src/fredlib/object/get_states_presence.h"

namespace Fred {
namespace MojeID {
namespace Check {

states_before_transfer_into_mojeid::states_before_transfer_into_mojeid(
    const InfoContactData &_data,
    OperationContext &_ctx)
{
    using namespace Fred::Object;
    typedef State::set< State::SERVER_TRANSFER_PROHIBITED,
                        State::SERVER_UPDATE_PROHIBITED,
                        State::SERVER_DELETE_PROHIBITED,
                        State::MOJEID_CONTACT,
                        State::CONDITIONALLY_IDENTIFIED_CONTACT,
                        State::IDENTIFIED_CONTACT >::type SetOfStates;
    const Get< Type::CONTACT >::States< SetOfStates >::Presence states_presence =
        Get< Type::CONTACT >(_data.id).states< SetOfStates >().presence(_ctx);
    server_transfer_prohibited_present = states_presence.get< State::SERVER_TRANSFER_PROHIBITED >();
    server_update_prohibited_present   = states_presence.get< State::SERVER_UPDATE_PROHIBITED >();
    server_delete_prohibited_present   = states_presence.get< State::SERVER_DELETE_PROHIBITED >();
    mojeid_contact_present             = states_presence.get< State::MOJEID_CONTACT >();
    no_identified_contact_present    = !(states_presence.get< State::CONDITIONALLY_IDENTIFIED_CONTACT >() ||
                                         states_presence.get< State::IDENTIFIED_CONTACT >());
}

bool states_before_transfer_into_mojeid::success()const
{
    return !(server_transfer_prohibited_present ||
             server_update_prohibited_present ||
             server_delete_prohibited_present ||
             mojeid_contact_present ||
             no_identified_contact_present);
}

}//Fred::MojeID::Check
}//Fred::MojeID
}//Fred
