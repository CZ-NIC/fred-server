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
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include "src/fredlib/opcontext.h"
#include "src/fredlib/zone/zone.h"
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

    if(fqdn.empty()) {
        return false;//we need some domain
    }
    if(*fqdn.begin() == '.') {
        return false;//fqdn have to start with label
    }
    //full domain name length, is limited to 255 octets
    //every label including the root label need one octet for label length
    //number of length octets corresponds to number of '.' separators
    //when fqdn ends with '.' it need one extra octet for length of the root label
    //when fqdn don't ends with '.' it need two extra octet for length of last label and length of the root label
    if(*fqdn.rbegin() == '.' ? fqdn.length() > 254 : fqdn.length() > 253) {
        return false;
    }

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

bool domain_name_ldh_and_no_label_beginning_or_ending_with_hyphen_syntax_check(const std::string& fqdn) {
    const boost::regex rfc1123_2_1_name_syntax(
            "^([[:alnum:]]([-[:alnum:]]*[[:alnum:]])*[.]?)+$");
    return boost::regex_match(fqdn, rfc1123_2_1_name_syntax);

}

bool domain_name_rfc1123_2_1_syntax_check(const std::string& fqdn) {
    return
        general_domain_name_syntax_check(fqdn) &&
        domain_name_ldh_and_no_label_beginning_or_ending_with_hyphen_syntax_check(fqdn);
}

DomainNameValidator::DomainNameValidator(const bool _is_system_registrar)
    : is_system_registrar_(_is_system_registrar)
{ }

//domain name validator
DomainNameValidator& DomainNameValidator::set_zone_name(const DomainName& _zone_name) {
    zone_name_.reset(new DomainName(_zone_name));

    return *this;
}

DomainNameValidator& DomainNameValidator::set_ctx(Fred::OperationContext& _ctx) {
    ctx_= &_ctx;

    return *this;
}


void DomainName::init(const char* const _fqdn) {
    if(_fqdn == NULL) {
        throw ExceptionInvalidFqdn();
    }
    std::string temp_fqdn(_fqdn);

    if( general_domain_name_syntax_check (temp_fqdn) == false) {
        throw ExceptionInvalidFqdn();
    }

    temp_fqdn = Fred::Zone::rem_trailing_dot(temp_fqdn);
    boost::split(labels_,temp_fqdn , boost::is_any_of("."));
}


DomainName::DomainName(const std::string& _fqdn) {
    init(_fqdn.c_str());
}

DomainName::DomainName(const char* const _fqdn) {
    init(_fqdn);
}

std::string DomainName::get_string() const {
    return boost::join(labels_, ".");
}

DomainName DomainName::get_subdomains(int _top_labels_to_skip) const {
    if( _top_labels_to_skip < 0 || _top_labels_to_skip > static_cast<int>(labels_.size()) ) {
        throw ExceptionInvalidLabelCount();
    }

    std::vector<std::string> selected_labels;

    selected_labels.assign( labels_.begin(), labels_.end() - _top_labels_to_skip );

    return DomainName(boost::join(selected_labels, "."));
}

DomainNameValidator& DomainNameValidator::add(const std::string& checker_name)
{
    FactoryHaveSupersetOfKeysChecker<Fred::Domain::DomainNameCheckerFactory>
        ::KeyVector required_keys = boost::assign::list_of(checker_name);
    FactoryHaveSupersetOfKeysChecker<Fred::Domain::DomainNameCheckerFactory>
        (required_keys).check();
    checker_name_vector_.push_back(checker_name);
    return *this;
}

DomainNameValidator& DomainNameValidator::set_checker_names(const std::vector<std::string>& checker_names)
{
    if (is_system_registrar_) {
        return *this;
    }

    for(std::vector<std::string>::const_iterator i = checker_names.begin(); i != checker_names.end() ; ++i){}
    FactoryHaveSupersetOfKeysChecker<Fred::Domain::DomainNameCheckerFactory>
        ::KeyVector required_keys = checker_names;
    FactoryHaveSupersetOfKeysChecker<Fred::Domain::DomainNameCheckerFactory>
        (required_keys).check();
    checker_name_vector_ = checker_names;
    return *this;
}


bool DomainNameValidator::exec(const DomainName& _fqdn, int top_labels_to_skip) {

    DomainName labels_to_check = _fqdn.get_subdomains(top_labels_to_skip);

    if (is_system_registrar_) {
        return true; // validation ok
    }

    for(std::vector<std::string>::const_iterator ci = checker_name_vector_.begin(); ci !=checker_name_vector_.end(); ++ci)
    {
        boost::shared_ptr<DomainNameChecker> checker = DomainNameCheckerFactory::instance_ref().create_sh_ptr(*ci);

        if(DomainNameCheckerNeedZoneName* need_zone_checker
                = dynamic_cast<DomainNameCheckerNeedZoneName*>(checker.get()))
        {
            if(zone_name_.get() == NULL) {
                throw ExceptionZoneNameNotSet();
            }
            need_zone_checker->set_zone_name(*zone_name_);
        }

        if(DomainNameCheckerNeedOperationContext* need_ctx_checker
                = dynamic_cast<DomainNameCheckerNeedOperationContext*>(checker.get()))
        {
            if(ctx_.isset() == false) {
                throw ExceptionCtxNotSet();
            }
            need_ctx_checker->set_ctx(*ctx_.get_value());
        }
        if(checker->validate(labels_to_check) == false) {
            return false; //validation failed
        }
    }
    return true;//validation ok
}

//trivial checker for testing
class DomainNameCheckerNotEmptyDomainName
: public DomainNameChecker
, public DomainNameCheckerNeedZoneName
, public DomainNameCheckerNeedOperationContext
, public Util::FactoryAutoRegister<DomainNameChecker, DomainNameCheckerNotEmptyDomainName>
{
    boost::scoped_ptr<DomainName> zone_name_;
    const Fred::OperationContext* ctx_ptr_;
public:
    DomainNameCheckerNotEmptyDomainName()
    : ctx_ptr_(0)
    {}

    bool validate(const DomainName& relative_domain_name)
    {
        return !zone_name_->get_string().empty() && !relative_domain_name.get_string().empty() && ctx_ptr_;
    }

    void set_zone_name(const DomainName& _zone_name)
    {
        zone_name_.reset(new DomainName(_zone_name) );
    }

    void set_ctx(const Fred::OperationContext& _ctx)
    {
        ctx_ptr_ = &_ctx;
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

    bool validate(const DomainName& relative_domain_name)
    {
        const boost::regex RFC1035_NAME_SYNTAX(
            "(([A-Za-z]|[A-Za-z][-A-Za-z0-9]{0,61}[A-Za-z0-9])[.])*"//optional non-highest-level labels
            "([A-Za-z]|[A-Za-z][-A-Za-z0-9]{0,61}[A-Za-z0-9])"//mandatory highest-level label
        );
        return boost::regex_match(relative_domain_name.get_string(), RFC1035_NAME_SYNTAX);
    }

    static std::string registration_name()
    {
        return DNCHECK_RFC1035_PREFERRED_SYNTAX;
    }
};//class CheckRFC1035PrefferedNameSyntax

///prohibit consecutive hyphens '--'
class CheckNoConsecutiveHyphens
: public DomainNameChecker
, public Util::FactoryAutoRegister<DomainNameChecker, CheckNoConsecutiveHyphens>
{
public:
    CheckNoConsecutiveHyphens(){}

    bool validate(const DomainName& relative_domain_name)
    {
        const boost::regex CONSECUTIVE_HYPHENS_SYNTAX("[-][-]");
        return !boost::regex_search(relative_domain_name.get_string(), CONSECUTIVE_HYPHENS_SYNTAX);
    }

    static std::string registration_name()
    {
        return DNCHECK_NO_CONSECUTIVE_HYPHENS;
    }
};//class CheckNoConsecutiveHyphensDomainName

///check domain name for single digit labels
class CheckSingleDigitLabelsOnly
: public DomainNameChecker
, public Util::FactoryAutoRegister<DomainNameChecker, CheckSingleDigitLabelsOnly>
{
public:
    CheckSingleDigitLabelsOnly(){}

    bool validate(const DomainName& relative_domain_name)
    {
        const boost::regex SINGLE_DIGIT_LABELS_SYNTAX(
            "([0-9][.])*"//optional non-highest-level single digit labels labels
            "[0-9]"//mandatory highest-level single digit label
        );
        return boost::regex_match(relative_domain_name.get_string(), SINGLE_DIGIT_LABELS_SYNTAX);
    }

    static std::string registration_name()
    {
        return DNCHECK_SINGLE_DIGIT_LABELS_ONLY;
    }
};//class CheckSingleDigitLabelsSyntax

///check that domain name labels contains only letters , digits or hyphens
///regardless of character position in the label
class CheckLettersDigitsHyphenCharsOnly
: public DomainNameChecker
, public Util::FactoryAutoRegister<DomainNameChecker, CheckLettersDigitsHyphenCharsOnly>
{
public:
    CheckLettersDigitsHyphenCharsOnly(){}

    bool validate(const DomainName& relative_domain_name)
    {
        const boost::regex DNCHECK_LETTER_DIGIT_HYPHEN_LABELS(
                "([-A-Za-z0-9]{1,63}[.])*"//optional non-highest-level labels
                "([-A-Za-z0-9]{1,63})"//mandatory highest-level label
        );
        return boost::regex_match(relative_domain_name.get_string(), DNCHECK_LETTER_DIGIT_HYPHEN_LABELS);
    }

    static std::string registration_name()
    {
        return DNCHECK_LETTERS_DIGITS_HYPHEN_CHARS_ONLY;
    }
};//class CheckSingleDigitLabelsSyntax

///check domain name for no hyphen at the start of the label
class CheckNoLabelBeginningHyphen
: public DomainNameChecker
, public Util::FactoryAutoRegister<DomainNameChecker, CheckNoLabelBeginningHyphen>
{
public:
    CheckNoLabelBeginningHyphen() {}

    bool validate(const DomainName& relative_domain_name)
    {
        //label starting by '-' prohibited
        if (!relative_domain_name.get_string().empty() && *relative_domain_name.get_string().begin() == '-') {
            return false;
        }
        const boost::regex NO_NEXT_START_HYPHEN_SYNTAX("[.][-]");
        return !boost::regex_search(relative_domain_name.get_string(), NO_NEXT_START_HYPHEN_SYNTAX);
    }

    static std::string registration_name()
    {
        return DNCHECK_NO_LABEL_BEGINNING_HYPHEN;
    }
};//class CheckNoStartHyphenSyntax

///check domain name for no hyphen at the end of the label
class CheckNoLabelEndingHyphen
: public DomainNameChecker
, public Util::FactoryAutoRegister<DomainNameChecker, CheckNoLabelEndingHyphen>
{
public:
    CheckNoLabelEndingHyphen() {}

    bool validate(const DomainName& relative_domain_name)
    {
        //label ending by '-' prohibited
        if (!relative_domain_name.get_string().empty() && *relative_domain_name.get_string().rbegin() == '-') {
            return false;
        }
        const boost::regex NO_NEXT_END_HYPHEN_SYNTAX("[-][.]");
        return !boost::regex_search(relative_domain_name.get_string(), NO_NEXT_END_HYPHEN_SYNTAX);
    }

    static std::string registration_name()
    {
        return DNCHECK_NO_LABEL_ENDING_HYPHEN;
    }
};//class CheckNoEndHyphenSyntax

///prohibit idn punycode ('xn--{punycode}')
class CheckNoIdnPunycode
: public DomainNameChecker
, public Util::FactoryAutoRegister<DomainNameChecker, CheckNoIdnPunycode>
{
public:
    CheckNoIdnPunycode(){}

    bool validate(const DomainName& relative_domain_name)
    {
        const std::string IDN_PUNYCODE_PREFIX("xn--");
        const std::vector<std::string> labels = relative_domain_name.get_labels(); // get labels (without zone labels)
        for (std::vector<std::string>::const_iterator label = labels.begin(); label != labels.end(); ++label) {
            if (boost::starts_with(*label, IDN_PUNYCODE_PREFIX)) {
                return false;
            }
        }
        return true;
    }

    static std::string registration_name()
    {
        return DNCHECK_NO_IDN_PUNYCODE;
    }
};//class CheckNoConsecutiveHyphensDomainName

void insert_domain_name_checker_name_into_database(Fred::OperationContext& ctx, const std::string& checker_name, const std::string& checker_description)
{
    ctx.get_conn().exec_params("INSERT INTO enum_domain_name_validation_checker(name, description)"
        " VALUES($1::text, $2::text)",Database::query_param_list(checker_name)(checker_description));
}

void set_domain_name_validation_config_into_database(Fred::OperationContext& ctx
    , const std::string& zone_name, const std::vector<std::string>& checker_names)
{
    Zone::Data zone = Zone::get_zone(ctx,zone_name);
    ctx.get_conn().exec_params("DELETE FROM zone_domain_name_validation_checker_map WHERE zone_id = $1::bigint"
        , Database::query_param_list(zone.id));
    for(std::vector<std::string>::const_iterator i = checker_names.begin(); i != checker_names.end(); ++i)
    {
        ctx.get_conn().exec_params("INSERT INTO zone_domain_name_validation_checker_map(zone_id, checker_id) "
            " VALUES($1::bigint, (SELECT id FROM enum_domain_name_validation_checker WHERE name = $2::text))",Database::query_param_list(zone.id)(*i));
    }//for checker_names
}

std::vector<std::string> get_domain_name_validation_config_for_zone(Fred::OperationContext& ctx
    , const std::string& zone_name)
{
    std::vector<std::string> checker_names;

    Database::Result checker_names_res = ctx.get_conn().exec_params(
        "SELECT ch.name FROM enum_domain_name_validation_checker ch "
        " JOIN zone_domain_name_validation_checker_map cfg ON ch.id = cfg.checker_id "
        " JOIN zone z ON z.id = cfg.zone_id "
        " WHERE z.fqdn = LOWER($1::text)", Database::query_param_list(zone_name));

    for(Database::Result::size_type i = 0 ; i < checker_names_res.size(); ++i)
    {
        checker_names.push_back(static_cast<std::string>(checker_names_res[i][0]));
    }
    return checker_names;
}



}//namespace Domain
}//namespace Fred

