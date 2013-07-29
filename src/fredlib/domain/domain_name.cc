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
#include <vector>
#include <boost/regex.hpp>
#include <boost/assign.hpp>

#include "fredlib/opcontext.h"
#include "fredlib/zone/zone.h"
#include "util/factory.h"
#include "util/factory_check.h"

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
    if(*fqdn.begin() == '.') return false;//fqdn have to start with label
    //full domain name length, is limited to 255 octets
    //every label including the root label need one octet for label length
    //number of length octets corresponds to number of '.' separators
    //when fqdn ends with '.' it need one extra octet for length of the root label
    //when fqdn don't ends with '.' it need two extra octet for length of last label and length of the root label
    if(*fqdn.rbegin() == '.' ? fqdn.length() > 254 : fqdn.length() > 253) return false;

    //check the length of labels
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

///check that domain name labels contains only letters , digits or hyphens
///regardless of character position in the label
class CheckLetterDigitHyphenLabelsSyntax
: public DomainNameChecker
, public Util::FactoryAutoRegister<DomainNameChecker, CheckLetterDigitHyphenLabelsSyntax>
{
public:
    CheckLetterDigitHyphenLabelsSyntax(){}

    bool validate(const std::string& relative_domain_name)
    {
        static const boost::regex DNCHECK_LETTER_DIGIT_HYPHEN_LABELS(
                "([-A-Za-z0-9]{1,63}[.])*"//optional non-highest-level labels
                "([-A-Za-z0-9]{1,63})"//mandatory highest-level label
        );
        return boost::regex_match(relative_domain_name, DNCHECK_LETTER_DIGIT_HYPHEN_LABELS);
    }

    static std::string registration_name()
    {
        return DNCHECK_LETTER_DIGIT_HYPHEN_LABELS;
    }
};//class CheckSingleDigitLabelsSyntax

///check domain name for no hyphen at the start of the label
class CheckNoStartHyphenSyntax
: public DomainNameChecker
, public Util::FactoryAutoRegister<DomainNameChecker, CheckNoStartHyphenSyntax>
{
public:
    CheckNoStartHyphenSyntax() {}

    bool validate(const std::string& relative_domain_name)
    {
        //label starting by '-' prohibited
        if (!relative_domain_name.empty() && *relative_domain_name.begin() == '-') return false;
        static const boost::regex NO_NEXT_START_HYPHEN_SYNTAX("[.][-]");
        return !boost::regex_search(relative_domain_name, NO_NEXT_START_HYPHEN_SYNTAX);
    }

    static std::string registration_name()
    {
        return DNCHECK_NO_START_HYPHEN_LABELS;
    }
};//class CheckNoStartHyphenSyntax

///check domain name for no hyphen at the end of the label
class CheckNoEndHyphenSyntax
: public DomainNameChecker
, public Util::FactoryAutoRegister<DomainNameChecker, CheckNoEndHyphenSyntax>
{
public:
    CheckNoEndHyphenSyntax() {}

    bool validate(const std::string& relative_domain_name)
    {
        //label ending by '-' prohibited
        if (!relative_domain_name.empty() && *relative_domain_name.rbegin() == '-') return false;
        static const boost::regex NO_NEXT_END_HYPHEN_SYNTAX("[-][.]");
        return !boost::regex_search(relative_domain_name, NO_NEXT_END_HYPHEN_SYNTAX);
    }

    static std::string registration_name()
    {
        return DNCHECK_NO_END_HYPHEN_LABELS;
    }
};//class CheckNoEndHyphenSyntax

void insert_domain_name_checker_name_into_database(Fred::OperationContext& ctx, const std::string& checker_name, const std::string& checker_description)
{
    ctx.get_conn().exec_params("INSERT INTO enum_domain_name_validation_checker(name, description)"
        " VALUES($1::text, $2::text)",Database::query_param_list(checker_name)(checker_description));
}

void set_domain_name_validation_config_into_database(Fred::OperationContext& ctx
    , const std::string& zone_name, const std::vector<std::string>& checker_names)
{
    Zone::Data zone = Zone::get_zone(ctx,zone_name);
    ctx.get_conn().exec_params("DELETE FROM domain_name_validation_config_by_zone WHERE zone_id = $1::bigint"
        , Database::query_param_list(zone.id));
    for(std::vector<std::string>::const_iterator i = checker_names.begin(); i != checker_names.end(); ++i)
    {
        ctx.get_conn().exec_params("INSERT INTO domain_name_validation_config_by_zone(zone_id, checker_id) "
            " VALUES($1::bigint, (SELECT id FROM enum_domain_name_validation_checker WHERE name = $2::text))",Database::query_param_list(zone.id)(*i));
    }//for checker_names
}




}//namespace Fred
}//namespace Domain

