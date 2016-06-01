/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
 */

#ifndef EPP_NSSET_DNS_HOST_DATA_06558ac1696645ffbbc1e22ce982b433
#define EPP_NSSET_DNS_HOST_DATA_06558ac1696645ffbbc1e22ce982b433

#include "src/epp/session_lang.h"
#include "src/fredlib/opcontext.h"
#include "util/db/nullable.h"

#include <string>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/ip/address.hpp>

namespace Epp {

    /**
     * DNS host data.
     */
    struct DNShostData {
        std::string fqdn;/**< nameserver host name*/
        std::vector<boost::asio::ip::address> inet_addr;/**< list of IPv4 or IPv6 addresses of the nameserver host*/

        /**
         * Constructor initializing all attributes.
         * @param _fqdn nameserver name
         * @param _inet_addr addresses of the nameserver
         */
        DNShostData(const std::string& _fqdn, const std::vector<boost::asio::ip::address>& _inet_addr)
        : fqdn(_fqdn)
        , inet_addr(_inet_addr)
        {}
    };

}

#endif
