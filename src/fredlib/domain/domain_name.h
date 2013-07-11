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
 *  @file domain_name.h
 *  domain name check
 */

#ifndef DOMAIN_NAME_H_
#define DOMAIN_NAME_H_

#include <string>
#include <boost/regex.hpp>
#include "fredlib/opcontext.h"
#include "util/factory.h"
#include "util/factory_check.h"

namespace Fred {
namespace Domain {

/**
 * GENERAL_DOMAIN_NAME_SYNTAX checking:
 * - labels length and root label according to RFC2181 section 11. Name syntax
 * - domain name preferred form using "LDH rule" according to RFC3696 section 2. Restrictions on domain (DNS) names
 * - allow to begin the non-highest-level label with digit
 *   according to RFC1123 section 2.1 Host Names and Numbers and because of ENUM RFC3761 section 2.4. Valid Databases
 *   otherwise it is syntax according to RFC1035 section 2.3.1. Preferred name syntax
 * - allow "IDNA Punycode" in any label according to RFC3696 section 5. Implications of internationalization
 */
const boost::regex GENERAL_DOMAIN_NAME_SYNTAX("^"//begin
    "(([A-Za-z0-9]|[A-Za-z0-9][-A-Za-z0-9]{0,61}[A-Za-z0-9])[.])+"//at least one non-highest-level label, may start with digit
        "([A-Za-z]|[A-Za-z][-A-Za-z0-9]{0,61}[A-Za-z0-9])"//mandatory highest-level label, have to begin with letter
        "[.]?$");//allowed optional root period
///using GENERAL_DOMAIN_NAME_SYNTAX and checking max overall length
bool general_domain_name_syntax_check(const std::string& fqdn);

/**
 * remove optional root dot from fqdn, domain names are considered fully qualified without trailing dot internally
 * optional root dot is required to be accepted by applications according to RFC3696 section 2.
 * but is not part of preferred name syntax RFC1035 section 2.3.1.
 */
std::string rem_trailing_dot(const std::string& fqdn);

//domain name validator
FACTORY_MODULE_INIT_DECL(domain_name_validator)


class DomainNameValidator
{
public:
    explicit DomainNameValidator(const std::string& fqdn);
    bool exec(const Fred::OperationContext& ctx);//fqdn valid/invalid

    //to be called from DomainNameCheckerBase child implementation
    Fred::OperationContext& get_op_ctx() const;
    std::string get_fqdn() const;
    unsigned long long get_zone_id() const;
};

class DomainNameCheckerBase
{
public:
  virtual ~DomainNameCheckerBase(){}
  virtual bool operator()(const DomainNameValidator& dnv) = 0;
};

const std::string DNCHECK_NO_DOUBLE_HYPHEN_UNICODE="dncheck_no_double_hyphen_unicode";
const std::string DNCHECK_NO_DOUBLE_HYPHEN_ASCII="dncheck_no_double_hyphen_ascii";
const std::string DNCHECK_RFC1035_PREFERRED_WITH_OPTIONAL_TRAILING_DOT="dncheck_rfc1035_preferred_with_optional_trailing_dot";
const std::string DNCHECK_ENUM="dncheck_enum";


}//namespace Fred
}//namespace Domain

#endif // DOMAIN_NAME_H_
