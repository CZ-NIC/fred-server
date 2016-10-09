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

#ifndef EPP_NSSET_IMPL_H_9e6a8e492e314c5993aec42a9eb4af7d
#define EPP_NSSET_IMPL_H_9e6a8e492e314c5993aec42a9eb4af7d

#include <vector>
#include <boost/asio/ip/address.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include "src/epp/nsset/nsset_dns_host_data.h"
#include "src/fredlib/nsset/nsset_dns_host.h"
#include "src/epp/nsset/nsset_dns_host.h"
#include "src/epp/error.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/zone/zone.h"

namespace Epp {

    bool is_prohibited_ip_addr(const boost::optional<boost::asio::ip::address>& ipaddr, Fred::OperationContext& ctx);
    std::vector<boost::asio::ip::address> make_ipaddrs(const std::vector<boost::optional<boost::asio::ip::address> >& inet_addr);
    std::vector<Fred::DnsHost> make_fred_dns_hosts(const std::vector<Epp::DNShostData>& data);
    std::vector<Epp::DNShost> make_epp_dns_hosts(const std::vector<Epp::DNShostData>& data);
    std::vector<Epp::DNShostData> make_epp_dnshosts_data(const std::vector<Fred::DnsHost>& data);

    //check that nameserver fqdn is in zone managed by registry to allow presence of nameserver IP addresses viz rfc1035#section-3.3.11
    template <class EXCEPTION> void check_disallowed_glue_ipaddrs(
        const Epp::DNShostData& nsdata, std::size_t current_nsset_ipaddr_position, EXCEPTION& ex, Fred::OperationContext& _ctx)
    {
        if(nsdata.inet_addr.size() > 0)
        {
            const std::string lower_dnshost_fqdn = boost::algorithm::to_lower_copy(nsdata.fqdn);
            try
            {
                Fred::Zone::find_zone_in_fqdn(_ctx,
                    Fred::Zone::rem_trailing_dot(lower_dnshost_fqdn));
            }
            catch(const Fred::Zone::Exception& e)
            {
                if(e.is_set_unknown_zone_in_fqdn()
                    && (e.get_unknown_zone_in_fqdn() == (Fred::Zone::rem_trailing_dot(lower_dnshost_fqdn)) ))
                {
                    //zone not found
                    std::size_t nsset_glue_ipaddr_to_position = current_nsset_ipaddr_position;
                    for(std::size_t j = 0; j < nsdata.inet_addr.size(); ++j, ++nsset_glue_ipaddr_to_position)
                    {
                        ex.add(Error::of_vector_parameter(Param::nsset_dns_addr,
                            boost::numeric_cast<unsigned short>(nsset_glue_ipaddr_to_position),
                            Reason::ip_glue_not_allowed));
                    }
                }
                else
                    throw;
            }
        }
    }

}

#endif
