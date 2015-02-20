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
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

class DiscloseFlagPolicy
: public boost::noncopyable
{
public:
    typedef boost::function<void(DiscloseFlagPolicy& policy)> PolicyCallback;
    typedef std::vector<PolicyCallback> PolicyCallbackVector;
private:
    Fred::Contact::Verification::Contact* contact_ptr_;
    PolicyCallbackVector policy_vect_;
public:
    DiscloseFlagPolicy(PolicyCallbackVector policy_vect);
    virtual ~DiscloseFlagPolicy(){}
    void append_policy_callback(PolicyCallback pc);
    void clear_policy_callback();
    void apply(Fred::Contact::Verification::Contact& contact);
    Fred::Contact::Verification::Contact& get_contact();
};

struct SetDiscloseAddrTrue
{
    void operator()(DiscloseFlagPolicy& policy);
};

struct SetDiscloseAddrTrueIfOrganization
{
    void operator()(DiscloseFlagPolicy& policy);
};

struct SetDiscloseAddrTrueIfNotIdentified
{
    void operator()(DiscloseFlagPolicy& policy);
};


#endif // MOJEID_DISCLOSE_POLICY_H_
