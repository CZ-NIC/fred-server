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
    if(fqdn.length() > 255) return false;
    return boost::regex_match(fqdn, GENERAL_DOMAIN_NAME_SYNTAX);
}

std::string rem_trailing_dot(const std::string& fqdn)
{
    if(!fqdn.empty() && fqdn.at(fqdn.size()-1) == '.') return fqdn.substr(0,fqdn.size()-1);
    return fqdn;
}

}//namespace Fred
}//namespace Domain

