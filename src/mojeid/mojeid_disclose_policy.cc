/*
 *  Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  @file
 *  mojeid disclose policy
 */

#include <string>
#include <vector>
#include <stdexcept>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/assign.hpp>

#include "src/fredlib/object_states.h"
#include "src/fredlib/contact_verification/contact.h"
#include "src/fredlib/contact_verification/contact_verification_state.h"
#include "src/mojeid/mojeid_contact_states.h"


#include "src/mojeid/mojeid_disclose_policy.h"

DiscloseFlagPolicy::DiscloseFlagPolicy(DiscloseFlagPolicy::PolicyCallbackVector policy_vect)
    :contact_ptr_(0)
    , policy_vect_(policy_vect)
{}

void DiscloseFlagPolicy::append_policy_callback(PolicyCallback pc)
{
    policy_vect_.push_back(pc);
}

void DiscloseFlagPolicy::clear_policy_callback()
{
    policy_vect_.clear();
}

void DiscloseFlagPolicy::apply(Fred::Contact::Verification::Contact& contact)
{
    contact_ptr_ = &contact;
    for(PolicyCallbackVector::iterator it = policy_vect_.begin()
        ; it != policy_vect_.end(); ++it)
    {
        if(*it) it->operator()(*this);
    }
}

Fred::Contact::Verification::Contact& DiscloseFlagPolicy::get_contact()
{
    if(contact_ptr_ == 0)
    {
        throw std::runtime_error("CznicDiscloseFlagPolicy::get_contact()"
            " error - pointer to contact not set");
    }
    return *contact_ptr_;
}

void SetDiscloseAddrTrue::operator()(DiscloseFlagPolicy& policy)
{
    policy.get_contact().discloseaddress=true;
}

void SetDiscloseAddrTrueIfOrganization::operator()(DiscloseFlagPolicy& policy)
{
    if(policy.get_contact().organization.get_value_or_default().compare("") != 0)
    {
        policy.get_contact().discloseaddress=true;
    }
}

void SetDiscloseAddrTrueIfNotIdentified::operator()(DiscloseFlagPolicy& policy)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result object_state_request_res = conn.exec_params(
        "SELECT "
        "COALESCE(BOOL_OR(eos.name='identifiedContact'),FALSE) as ic,"
        "COALESCE(BOOL_OR(eos.name='validatedContact'),FALSE) as vc "
        "FROM object_registry obr "
        "JOIN object o ON o.id=obr.id "
        "JOIN contact c ON c.id=o.id "
        "LEFT JOIN object_state_request osr ON ("
          "osr.object_id=obr.id AND ("
          "osr.valid_from<=CURRENT_TIMESTAMP AND ("
          "osr.valid_to ISNULL OR osr.valid_to>=CURRENT_TIMESTAMP)"
          " AND osr.canceled IS NULL)) "
        "LEFT JOIN enum_object_states eos ON eos.id=osr.state_id "
        "WHERE obr.id = $1::bigint "
        "GROUP BY obr.id"
    , Database::query_param_list(policy.get_contact().id));

    bool ic = false;
    bool vc = false;

    if(object_state_request_res.size() == 1)
    {
        ic = static_cast<bool>(object_state_request_res[0]["ic"]);
        vc = static_cast<bool>(object_state_request_res[0]["vc"]);
    }

    if(!(ic || vc))
    {
        LOGGER(PACKAGE).debug("SetDiscloseAddrTrueIfNotIdentified discloseaddress = true");
        policy.get_contact().discloseaddress = true;
    }
}

