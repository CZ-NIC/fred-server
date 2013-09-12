/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  @file
 *  nsset dns host
 */

#ifndef NSSET_DNS_HOST_H_
#define NSSET_DNS_HOST_H_

#include <string>
#include <vector>

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"

namespace Fred
{

    /**
     * Nameserver data container.
     */
    class DnsHost
    {
        std::string fqdn_;/**< fully qualified name of the nameserver host*/
        std::vector<std::string> inet_addr_;/**< list of IPv4 or IPv6 addresses of the nameserver host*/
    public:

        /**
         * Empty destructor.
         */
        virtual ~DnsHost(){}

        /**
         * Constructor initializing all attributes.
         * @param _fqdn sets nameserver name into @ref fqdn_ attribute
         * @param _inet_addr sets addresses of the nameserver into @ref inet_addr_ attribute.
         */
        DnsHost(const std::string& _fqdn, const std::vector<std::string>& _inet_addr)
        : fqdn_(_fqdn)
        , inet_addr_(_inet_addr)
        {}

        /**
         * Nameserver name getter.
         * @return name of nameserver viz @ref fqdn_
         */
        std::string get_fqdn() const
        {
            return fqdn_;
        }

        /**
         * Nameserver addresses getter.
         * @return addresses of nameserver field viz @ref inet_addr_
         */
        std::vector<std::string> get_inet_addr() const
        {
            return inet_addr_;
        }

        /**
         * Conversion to string.
         * @return textual description of the content
         */
        operator std::string() const
        {
            std::stringstream ret;
            ret << "DnsHost fqdn: " << fqdn_;
            if(!inet_addr_.empty()) ret << " inet_addr:";
            for(std::vector<std::string>::const_iterator ci = inet_addr_.begin()
                ; ci != inet_addr_.end(); ++ci) ret << " " << *ci;
            return ret.str();
        }

    };

}//namespace Fred

#endif//NSSET_DNS_HOST_H_
