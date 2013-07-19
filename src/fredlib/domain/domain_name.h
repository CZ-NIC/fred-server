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
#include <boost/shared_ptr.hpp>
#include "fredlib/opcontext.h"
#include "util/factory.h"
#include "util/factory_check.h"

namespace Fred {
namespace Domain {

///checking fqdn length < 255 and label length is from 1 to 63 octets, labels are separated by '.'
bool general_domain_name_syntax_check(const std::string& fqdn);

/**
 * remove optional root dot from fqdn, domain names are considered fully qualified without trailing dot internally
 * optional root dot is required to be accepted by applications according to RFC3696 section 2.
 * but is not part of preferred name syntax RFC1035 section 2.3.1.
 */
std::string rem_trailing_dot(const std::string& fqdn);

//domain name validator
FACTORY_MODULE_INIT_DECL(domain_name_validator)

class DomainNameChecker
{
public:
  virtual ~DomainNameChecker(){}
  virtual bool validate(const std::string& domain_name) = 0;
};

class DomainNameCheckerNeedZoneName
{
public:
    virtual void set_zone_name(const std::string& zone_name) = 0;
protected:
   ~DomainNameCheckerNeedZoneName(){}
};

class DomainNameCheckerNeedOperationContext
{
public:
    virtual void set_ctx(const Fred::OperationContext& ctx) = 0;
protected:
   ~DomainNameCheckerNeedOperationContext(){}
};

class DomainNameValidator
{
    const std::string relative_domain_name_;
    const std::string zone_name_;
    std::vector<std::string> checker_name_vector_;
public:
    ///domain name is considered relative and zone name is considered fully qualified here
    ///, so no root dot at the end of strings are expected
    ///relative_domain_name is validated by checkers
    ///zone_name begins with label and may be empty if considered unimportant for checkers
    ///zone_name may be used by implementation of DomainNameChecker
    ///if zone_name is empty and checker implementation need some value, validation shall fail
    explicit DomainNameValidator(const std::string& relative_domain_name, const std::string& zone_name = "");
    ///add checker instance shared pointer
    DomainNameValidator& operator()(const std::string& checker_name);
    ///returns true if domain name is valid otherwise it returns false
    bool exec(const Fred::OperationContext& ctx);
};

typedef Util::Factory<DomainNameChecker, Util::ClassCreator<DomainNameChecker> > DomainNameCheckerFactory;

const std::string DNCHECK_NO_CONSECUTIVE_HYPHENS="dncheck_no_consecutive_hyphens";
const std::string DNCHECK_RFC1035_PREFERRED_SYNTAX="dncheck_rfc1035_preferred_syntax";
const std::string DNCHECK_SINGLE_DIGIT_LABELS="dncheck_single_digit_labels";
const std::string DNCHECK_LETTER_DIGIT_HYPHEN_LABELS="dncheck_letter_digit_hyphen_labels";

//trivial checker for testing
const std::string DNCHECK_NOT_EMPTY_DOMAIN_NAME="dncheck_not_empty_domain_name";

}//namespace Fred
}//namespace Domain

#endif // DOMAIN_NAME_H_
