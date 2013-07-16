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
#include <boost/assign.hpp>

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
DomainNameValidator::DomainNameValidator(const std::string& relative_domain_name, const std::string& zone_name)
: relative_domain_name_(relative_domain_name), zone_name_(zone_name)
{}

DomainNameValidator& DomainNameValidator::operator()(const std::string& checker_name)
{
    FactoryHaveSupersetOfKeysChecker<Fred::Domain::DomainNameCheckerFactory>
        ::KeyVector required_keys = boost::assign::list_of(checker_name);
    FactoryHaveSupersetOfKeysChecker<Fred::Domain::DomainNameCheckerFactory>
        (required_keys).check();
    checker_name_vector_.push_back(checker_name);
    return *this;
}

bool DomainNameValidator::exec(const Fred::OperationContext& ctx)
{
    if(!zone_name_.empty() && *(--zone_name_.end()) == '.') return false; //unexpected root dot
    if(general_domain_name_syntax_check(relative_domain_name_
        +(zone_name_.empty() ? std::string("") : std::string(".")+zone_name_)) == false)
    {
        return false;
    }

    for(std::vector<std::string>::const_iterator ci = checker_name_vector_.begin(); ci !=checker_name_vector_.end(); ++ci)
    {
        boost::shared_ptr<DomainNameChecker> checker = DomainNameCheckerFactory::instance_ref().create_sh_ptr(*ci);

        if(DomainNameCheckerNeedZoneName* need_zone_checker
                = dynamic_cast<DomainNameCheckerNeedZoneName*>(checker.get()))
        {
            need_zone_checker->set_zone_name(zone_name_);
        }

        if(DomainNameCheckerNeedOperationContext* need_ctx_checker
                = dynamic_cast<DomainNameCheckerNeedOperationContext*>(checker.get()))
        {
            need_ctx_checker->set_ctx(ctx);
        }
        if(checker.get()->validate(relative_domain_name_) == false) return false; //validation failed
    }//for checker_name_vector_
    // check

    return true;//validation ok
}

//trivial checker for testing
class DomainNameCheckerNotEmptyDomainName
: public DomainNameChecker
, public DomainNameCheckerNeedZoneName
, public DomainNameCheckerNeedOperationContext
, public Util::FactoryAutoRegister<DomainNameChecker, DomainNameCheckerNotEmptyDomainName>
{
    std::string zone_name_;
    const Fred::OperationContext* ctx_ptr_;
public:
    DomainNameCheckerNotEmptyDomainName()
    : ctx_ptr_(0)
    {}

    bool validate(const std::string& relative_domain_name)
    {
        return !zone_name_.empty() && !relative_domain_name.empty() && ctx_ptr_;
    }

    void set_zone_name(const std::string& zone_name)
    {
        zone_name_ = zone_name;
    }

    void set_ctx(const Fred::OperationContext& ctx)
    {
        ctx_ptr_ = &ctx;
    }

    static std::string registration_name()
    {
        return DNCHECK_NOT_EMPTY_DOMAIN_NAME;
    }
};//class DomainNameCheckerNotEmptyDomainName


///check domain name according to RFC1035 section 2.3.1. Preferred name syntax
class CheckRFC1035PreferredNameSyntax
: public DomainNameChecker
, public Util::FactoryAutoRegister<DomainNameChecker, CheckRFC1035PreferredNameSyntax>
{
public:
    CheckRFC1035PreferredNameSyntax()
    {}

    bool validate(const std::string& relative_domain_name)
    {
        static const boost::regex RFC1035_NAME_SYNTAX(
            "(([A-Za-z]|[A-Za-z][-A-Za-z0-9]{0,61}[A-Za-z0-9])[.])*"//optional non-highest-level labels
            "([A-Za-z]|[A-Za-z][-A-Za-z0-9]{0,61}[A-Za-z0-9])"//mandatory highest-level label
        );
        return boost::regex_match(relative_domain_name, RFC1035_NAME_SYNTAX);
    }

    static std::string registration_name()
    {
        return DNCHECK_RFC1035_PREFERRED_SYNTAX;
    }
};//class CheckRFC1035PrefferedNameSyntax

///prohibit consecutive hyphens '--'
class CheckNoConsecutiveHyphensDomainName
: public DomainNameChecker
, public Util::FactoryAutoRegister<DomainNameChecker, CheckNoConsecutiveHyphensDomainName>
{
public:
    CheckNoConsecutiveHyphensDomainName(){}

    bool validate(const std::string& relative_domain_name)
    {
        static const boost::regex CONSECUTIVE_HYPHENS_SYNTAX("[-][-]");
        return !boost::regex_search(relative_domain_name, CONSECUTIVE_HYPHENS_SYNTAX);
    }

    static std::string registration_name()
    {
        return DNCHECK_NO_CONSECUTIVE_HYPHENS;
    }
};//class CheckNoConsecutiveHyphensDomainName

///check domain name for single digit labels
class CheckSingleDigitLabelsSyntax
: public DomainNameChecker
, public Util::FactoryAutoRegister<DomainNameChecker, CheckSingleDigitLabelsSyntax>
{
public:
    CheckSingleDigitLabelsSyntax(){}

    bool validate(const std::string& relative_domain_name)
    {
        static const boost::regex SINGLE_DIGIT_LABELS_SYNTAX(
            "([0-9][.])*"//optional non-highest-level single digit labels labels
            "[0-9]"//mandatory highest-level single digit label
        );
        return boost::regex_match(relative_domain_name, SINGLE_DIGIT_LABELS_SYNTAX);
    }

    static std::string registration_name()
    {
        return DNCHECK_SINGLE_DIGIT_LABELS;
    }
};//class CheckSingleDigitLabelsSyntax


}//namespace Fred
}//namespace Domain

