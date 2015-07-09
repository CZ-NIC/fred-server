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
 *  header of mojeid2 checkers
 */

#ifndef MOJEID2_CHECKERS_H_3F3071E19E8ADD1CB7438F97BBE37530//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define MOJEID2_CHECKERS_H_3F3071E19E8ADD1CB7438F97BBE37530

#include "src/fredlib/contact/checkers.h"
#include "src/fredlib/contact/info_contact_data.h"
#include "src/fredlib/object/get_states_presence.h"

namespace Fred {
namespace MojeID {
namespace Check {

struct new_contact_email_availability:GeneralCheck::contact_email_availability
{
    enum { UNUSED_CONTACT_ID = 0 };
    new_contact_email_availability(const InfoContactData &_data, OperationContext &_ctx)
    :   GeneralCheck::contact_email_availability(_data.email, UNUSED_CONTACT_ID, _ctx)
    { }
};

struct new_contact_phone_availability:GeneralCheck::contact_phone_availability
{
    enum { UNUSED_CONTACT_ID = 0 };
    new_contact_phone_availability(const InfoContactData &_data, OperationContext &_ctx)
    :   GeneralCheck::contact_phone_availability(_data.telephone, UNUSED_CONTACT_ID, _ctx)
    { }
};

template < typename STATES_PRESENCE >
struct states_before_transfer_into_mojeid
{
    states_before_transfer_into_mojeid(const STATES_PRESENCE &_states_presence)
    :   server_transfer_prohibited_present(_states_presence.template get< Fred::Object::State::SERVER_TRANSFER_PROHIBITED >()),
        server_update_prohibited_present  (_states_presence.template get< Fred::Object::State::SERVER_UPDATE_PROHIBITED >()),
        server_delete_prohibited_present  (_states_presence.template get< Fred::Object::State::SERVER_DELETE_PROHIBITED >()),
        mojeid_contact_present            (_states_presence.template get< Fred::Object::State::MOJEID_CONTACT >())
    { }
    bool success()const
    {
        return !(server_transfer_prohibited_present ||
                 server_update_prohibited_present ||
                 server_delete_prohibited_present ||
                 mojeid_contact_present);
    }
    bool server_transfer_prohibited_present:1;
    bool server_update_prohibited_present:1;
    bool server_delete_prohibited_present:1;
    bool mojeid_contact_present:1;
};

}//Fred::MojeID::Check
}//Fred::MojeID
}//Fred

#endif//MOJEID2_CHECKERS_H_3F3071E19E8ADD1CB7438F97BBE37530
