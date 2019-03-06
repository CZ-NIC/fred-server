/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef NSSET_HH_33B9C4D3463347E98F1D3C6CDA721323
#define NSSET_HH_33B9C4D3463347E98F1D3C6CDA721323

#include "src/backend/epp/epp_extended_error.hh"
#include "src/backend/epp/param.hh"
#include "src/backend/epp/reason.hh"
#include "src/backend/epp/nsset/dns_host_input.hh"
#include "src/backend/epp/nsset/dns_host_output.hh"
#include "libfred/registrable_object/nsset/nsset_dns_host.hh"
#include "libfred/opexception.hh"
#include "libfred/zone/zone.hh"

#include <boost/asio/ip/address.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Nsset {

bool is_prohibited_ip_addr(
        const boost::optional<boost::asio::ip::address>& ipaddr,
        LibFred::OperationContext& ctx);


std::vector<boost::asio::ip::address> make_ipaddrs(
        const std::vector<boost::optional<boost::asio::ip::address> >& inet_addr);


std::vector<LibFred::DnsHost> make_fred_dns_hosts(const std::vector<DnsHostInput>& data);


std::vector<DnsHostOutput> make_epp_dnshosts_output(const std::vector<LibFred::DnsHost>& data);


// check that nameserver fqdn is in zone managed by registry to allow presence of nameserver IP addresses viz rfc1035#section-3.3.11
template <class T>
void check_disallowed_glue_ipaddrs(
        const DnsHostInput& _nsdata,
        std::size_t _current_nsset_ipaddr_position,
        T& _parameter_value_policy_error,
        LibFred::OperationContext& _ctx)
{
    if (_nsdata.inet_addr.size() > 0)
    {
        const std::string lower_dnshost_fqdn = boost::algorithm::to_lower_copy(_nsdata.fqdn);
        try
        {
            LibFred::Zone::find_zone_in_fqdn(
                    _ctx,
                    LibFred::Zone::rem_trailing_dot(lower_dnshost_fqdn));
        }
        catch (const LibFred::Zone::Exception& e)
        {
            if (e.is_set_unknown_zone_in_fqdn()
                && (e.get_unknown_zone_in_fqdn() == (LibFred::Zone::rem_trailing_dot(lower_dnshost_fqdn))))
            {
                // zone not found
                std::size_t nsset_glue_ipaddr_to_position = _current_nsset_ipaddr_position;
                for (std::size_t j = 0; j < _nsdata.inet_addr.size(); ++j, ++nsset_glue_ipaddr_to_position)
                {
                    // ex.add(Error::of_vector_parameter(Param::nsset_dns_addr,
                    //    boost::numeric_cast<unsigned short>(nsset_glue_ipaddr_to_position),
                    //    Reason::ip_glue_not_allowed));
                    _parameter_value_policy_error.add_extended_error(
                            EppExtendedError::of_vector_parameter(
                                    Param::nsset_dns_addr,
                                    boost::numeric_cast<unsigned short>(nsset_glue_ipaddr_to_position),
                                    Reason::ip_glue_not_allowed));
                }
            }
            else
            {
                throw;
            }
        }
    }
}


} // namespace Epp::Nsset
} // namespace Epp

#endif
