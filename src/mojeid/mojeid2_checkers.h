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
#include "src/mojeid/mojeid2.h"

namespace Fred {
namespace MojeID {
namespace Check {

struct contact_username_availability:GeneralCheck::MojeID::contact_username_availability
{
    contact_username_availability(const CreateContact &_data, OperationContext &_ctx)
    :   GeneralCheck::MojeID::contact_username_availability(_data.username, _ctx)
    { }
};

struct contact_name:GeneralCheck::contact_name
{
    contact_name(const CreateContact &_data)
    :   GeneralCheck::contact_name(_data.first_name, _data.last_name)
    { }
};

struct contact_address:GeneralCheck::contact_address
{
    contact_address(const Address &_data)
    :   GeneralCheck::contact_address(
            _data.street1,
            _data.city,
            _data.postal_code,
            _data.country)
    { }
};

struct contact_permanent_address:contact_address
{
    contact_permanent_address(const CreateContact &_data)
    :   contact_address(_data.permanent)
    { }
};

struct contact_email_presence:GeneralCheck::contact_email_presence
{
    contact_email_presence(const CreateContact &_data)
    :   GeneralCheck::contact_email_presence(_data.email)
    { }
};

struct contact_email_validity:GeneralCheck::contact_email_validity
{
    contact_email_validity(const CreateContact &_data)
    :   GeneralCheck::contact_email_validity(_data.email)
    { }
};

struct contact_email_availability:GeneralCheck::contact_email_availability
{
    enum { UNUSED_CONTACT_ID = 0 };
    contact_email_availability(const CreateContact &_data, OperationContext &_ctx)
    :   GeneralCheck::contact_email_availability(_data.email, UNUSED_CONTACT_ID, _ctx)
    { }
};

struct contact_phone_presence:GeneralCheck::contact_phone_presence
{
    contact_phone_presence(const CreateContact &_data)
    :   GeneralCheck::contact_phone_presence(_data.teplephone)
    { }
};

struct contact_phone_validity:GeneralCheck::contact_phone_validity
{
    contact_phone_validity(const CreateContact &_data)
    :   GeneralCheck::contact_phone_validity(_data.teplephone)
    { }
};

struct contact_phone_availability:GeneralCheck::contact_phone_availability
{
    enum { UNUSED_CONTACT_ID = 0 };
    contact_phone_availability(const CreateContact &_data, OperationContext &_ctx)
    :   GeneralCheck::contact_phone_availability(_data.teplephone, UNUSED_CONTACT_ID, _ctx)
    { }
};

}//Fred::MojeID::Check
}//Fred::MojeID
}//Fred

#endif//MOJEID2_CHECKERS_H_3F3071E19E8ADD1CB7438F97BBE37530
