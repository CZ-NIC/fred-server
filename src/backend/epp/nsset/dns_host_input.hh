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

#ifndef DNS_HOST_INPUT_HH_A3F8D3477B9140BBA679D6745244ADD8
#define DNS_HOST_INPUT_HH_A3F8D3477B9140BBA679D6745244ADD8

#include "src/backend/epp/session_lang.hh"
#include "libfred/opcontext.hh"
#include "util/db/nullable.hh"

#include <boost/asio/ip/address.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include <string>
#include <vector>

namespace Epp {
namespace Nsset {

/**
 * DNS host data.
 */
struct DnsHostInput
{
    std::string fqdn; /**< nameserver host name */
    std::vector<boost::optional<boost::asio::ip::address> > inet_addr; /**< list of IPv4 or IPv6 addresses of the nameserver host, non-initialized if value is invalid */


    /**
     * Constructor initializing all attributes.
     * @param _fqdn nameserver name
     * @param _inet_addr addresses of the nameserver, non-initialized if value is invalid
     */
    DnsHostInput(
            const std::string& _fqdn,
            const std::vector<boost::optional<boost::asio::ip::address> >& _inet_addr)
        : fqdn(_fqdn),
          inet_addr(_inet_addr)
    {
    }


};

} // namespace Epp::Nsset
} // namespace Epp

#endif
