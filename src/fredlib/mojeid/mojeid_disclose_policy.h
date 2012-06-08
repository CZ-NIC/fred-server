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
#include <vector>
#include <stdexcept>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/assign.hpp>

#include "fredlib/object_states.h"
#include "fredlib/mojeid/contact.h"
#include "fredlib/mojeid/mojeid_contact_states.h"

class DiscloseFlagPolicy
: public boost::noncopyable
{
public:
    typedef boost::function<void(DiscloseFlagPolicy& policy)> PolicyCallback;
    typedef std::vector<PolicyCallback> PolicyCallbackVector;
private:
    ::MojeID::Contact* contact_ptr_;
    PolicyCallbackVector policy_vect_;
public:

    DiscloseFlagPolicy(PolicyCallbackVector policy_vect)
    :contact_ptr_(0)
    , policy_vect_(policy_vect)
    {}

    virtual ~DiscloseFlagPolicy(){}

    void append_policy_callback(PolicyCallback pc)
    {
        policy_vect_.push_back(pc);
    }

    void clear_policy_callback()
    {
        policy_vect_.clear();
    }


    void apply(::MojeID::Contact& contact)
    {
        contact_ptr_ = &contact;
        for(PolicyCallbackVector::iterator it = policy_vect_.begin()
            ; it != policy_vect_.end(); ++it)
        {
            if(*it) it->operator()(*this);
        }

    }

    ::MojeID::Contact& get_contact()
    {
        if(contact_ptr_ == 0)
        {
            throw std::runtime_error("CznicDiscloseFlagPolicy::get_contact()"
                " error - pointer to contact not set");
        }
        return *contact_ptr_;
    }
};

struct SetDiscloseAddrTrue
{
    void operator()(DiscloseFlagPolicy& policy)
    {
        policy.get_contact().discloseaddress=true;
    }
};

struct SetDiscloseAddrTrueIfOrganization
{
    void operator()(DiscloseFlagPolicy& policy)
    {
        if(std::string(policy.get_contact().organization).compare("") != 0)
        {
            policy.get_contact().discloseaddress=true;
        }
    }
};

struct SetDiscloseAddrTrueIfNotValidated
{
    void operator()(DiscloseFlagPolicy& policy)
    {
        if (Fred::object_has_state(policy.get_contact().id
                , ::MojeID::VALIDATED_CONTACT) == false)
        {
            policy.get_contact().discloseaddress=true;
        }
    }
};


#endif // MOJEID_DISCLOSE_POLICY_H_
