/*
 *  Copyright (C) 2012  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file mojeid_disclose_policy.h
 *  disclose policy
 */
#ifndef MOJEID_DISCLOSE_POLICY_H_
#define MOJEID_DISCLOSE_POLICY_H_

#include <string>
#include <boost/noncopyable.hpp>

#include "fredlib/object_states.h"
#include "fredlib/mojeid/contact.h"
#include "fredlib/mojeid/mojeid_contact_states.h"

class CznicDiscloseFlagPolicy
: public boost::noncopyable
{
public:
    void apply(::MojeID::Contact& contact)
    {
        if((Fred::object_has_state(contact.id
                , ::MojeID::VALIDATED_CONTACT) == false)
            || !contact.organization.isnull()
            || std::string(contact.organization).compare("") != 0)
        {
            contact.discloseaddress=true;
        }
    }
};

#endif // MOJEID_DISCLOSE_POLICY_H_
