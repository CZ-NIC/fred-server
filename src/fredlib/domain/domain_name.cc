/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  @file domain_name.cc
 *  domain name check
 */


#include "domain_name.h"

#include <string>
#include <boost/regex.hpp>

namespace Fred {
namespace Domain {

FACTORY_MODULE_INIT_DEFI(domain_name_validator)

bool general_domain_name_syntax_check(const std::string& fqdn)
{
    /* general domain name syntax check as specified
    in: RFC1034, RFC1035, RFC2181 and RFC3696
    label may contain anything, except of label separator '.'
    in case of need to have '.' in a label, there have to be some kind of escaping
    e.g. RFC4343 section 2.1. Escaping Unusual DNS Label Octets */

    if(fqdn.empty()) return false;//we need some domain
    if(fqdn.length() > 255) return false; //full domain name length, including the separators, is limited to 255 octets
    if(fqdn.at(0) == '.') return false;//fqdn have to start with label
    unsigned long long label_octet_counter = 0;
    for(std::string::const_iterator i = fqdn.begin(); i != fqdn.end(); ++i)
    {
        if(*i == '.')
        {//found separator
            if((label_octet_counter < 1) //label is too short
                || (label_octet_counter > 63))//or label is too long
            {
                return false;
            }
            else
            {
                label_octet_counter = 0;
            }
        }
        else
        {//found label character
            ++label_octet_counter;
        }
    }//for fqdn
    //if fqdn ends with optional root dot then label_octet_counter equals 0 at the end, ok
    if(label_octet_counter > 63)//or label is too long
    {
        return false;
    }

    return true;
}

std::string rem_trailing_dot(const std::string& fqdn)
{
    if(!fqdn.empty() && fqdn.at(fqdn.size()-1) == '.') return fqdn.substr(0,fqdn.size()-1);
    return fqdn;
}

//domain name validator
DomainNameValidator::DomainNameValidator(const std::string& fqdn)
: ctx_ptr_(0), fqdn_(fqdn), zone_id_(0)
{

}

bool DomainNameValidator::exec(const Fred::OperationContext& ctx)
{
    ctx_ptr_ = &ctx;
    if(general_domain_name_syntax_check(fqdn_) == false) return false;

    return true;
}

const Fred::OperationContext& DomainNameValidator::get_op_ctx() const
{
    if (ctx_ptr_ != 0)
    {
        return *ctx_ptr_;
    }
    else
    {
        throw std::runtime_error("DomainNameValidator::get_op_ctx have no OperationContext");
    }
}
const std::string DomainNameValidator::get_fqdn() const
{
    return fqdn_;
}
unsigned long long DomainNameValidator::get_zone_id() const
{
    return zone_id_;
}

}//namespace Fred
}//namespace Domain

