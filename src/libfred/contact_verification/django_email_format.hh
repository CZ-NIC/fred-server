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

#ifndef DJANGO_EMAIL_FORMAT_HH_5AD46276489949D5B6F5171D07263344
#define DJANGO_EMAIL_FORMAT_HH_5AD46276489949D5B6F5171D07263344

#include "src/util/optional_value.hh"
#include "src/util/idn_utils.hh"

#include <boost/regex.hpp>
#include <string>
#include <algorithm>


/**
 * should work the same as django validator viz https://github.com/django/django/blob/1.6.9/django/core/validators.py#L80-L124
 */
class DjangoEmailFormat
{
    const std::vector<std::string> domain_whitelist_;

public:
    DjangoEmailFormat(const std::vector<std::string>& domain_whitelist = std::vector<std::string>())
    : domain_whitelist_(domain_whitelist)
    {}

    bool check(const std::string& email )
    {
        static const boost::regex user_regex("(^[-!#$%&'*+/=?^_`{}|~0-9A-Z]+(\\.[-!#$%&'*+/=?^_`{}|~0-9A-Z]+)*$" //dot-atom
            "|^\"([\\0001-\\0010\\0013\\0014\\0016-\\0037!#-\\[\\]-\\0177]|\\\\[\\0001-\\0011\\0013\\0014\\0016-\\0177])*\"$)" //quoted-string
            , boost::regex::icase);
        static const boost::regex domain_regex("(?:[A-Z0-9](?:[A-Z0-9-]{0,61}[A-Z0-9])?\\.)+(?:[A-Z]{2,6}|[A-Z0-9-]{2,}(?<!-))$"
            "|^\\[(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)(\\.(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)){3}\\]$"// literal form, ipv4 address (SMTP 4.1.3)
            , boost::regex::icase);

        if(email.empty()) return false;

        std::string::size_type email_at = email.rfind('@');
        if (email_at == std::string::npos) return false;

        try
        {
            std::string user_part = email.substr(0,email_at);
            if(user_part.empty()) return false;

            std::string domain_part = email.substr(email_at+1);
            if(domain_part.empty()) return false;

            if (!boost::regex_match(user_part,user_regex)) return false;

            if(std::find(domain_whitelist_.begin(), domain_whitelist_.end()
                , domain_part)!=domain_whitelist_.end())
            {
                return true;
            }

            if (!boost::regex_match(domain_part,domain_regex))
            {
                //domain part idn conversion
                std::string converted_domain_part = Util::convert_utf8_to_punnycode(domain_part).get_value_or_default();

                if(converted_domain_part.empty()) return false;

                if(!boost::regex_match(converted_domain_part,domain_regex)) return false;
            }
        }
        catch(const std::exception& ex)
        {
            return false;
        }
        return true;
    };

};

#endif
