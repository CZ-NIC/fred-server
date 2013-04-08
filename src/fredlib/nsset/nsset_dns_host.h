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
 *  @file nsset_dns_host.h
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

class DnsHost
    {
        std::string fqdn_;
        std::vector<std::string> inet_addr_;
    public:
        virtual ~DnsHost(){}
        DnsHost(const std::string& _fqdn, const std::vector<std::string>& _inet_addr)
        : fqdn_(_fqdn)
        , inet_addr_(_inet_addr)
        {}

        std::string get_fqdn() const
        {
            return fqdn_;
        }

        std::vector<std::string> get_inet_addr() const
        {
            return inet_addr_;
        }
    };

}//namespace Fred

#endif//NSSET_DNS_HOST_H_
