#include "src/epp/nsset/impl/nsset.h"
#include <boost/foreach.hpp>

namespace Epp {
namespace Nsset {

    bool is_prohibited_ip_addr(
        const boost::optional<boost::asio::ip::address>& ipaddr,
        Fred::OperationContext& ctx)
    {
        if (!ipaddr.is_initialized())
        {
            return true;
        }

        Database::Result ip_check = ctx.get_conn().exec_params(
            "SELECT bool_or(($1::inet & netmask) = network) "
            " FROM nsset_dnshost_prohibited_ipaddr"
            " WHERE family($1::inet) = family(network)",
            Database::query_param_list(ipaddr.get().to_string()));

        if(ip_check.size() > 1)
        {
            throw std::logic_error("wrong sql query");
        }

        if((ip_check.size() == 1) && static_cast<bool>(ip_check[0][0]))
        {
            return true;
        }

        return false;
    }

    std::vector<boost::asio::ip::address> make_ipaddrs(
        const std::vector<boost::optional<boost::asio::ip::address> >& data)
    {
        std::vector<boost::asio::ip::address> inet_addrs;
        inet_addrs.reserve(data.size());
        BOOST_FOREACH(const boost::optional<boost::asio::ip::address>& optional_ipaddr, data)
        {
            if(optional_ipaddr.is_initialized()) {
                inet_addrs.push_back(optional_ipaddr.get());
            }
        }
        return inet_addrs;
    }

    std::vector<boost::optional<boost::asio::ip::address> > make_optional_ipaddrs(
        const std::vector<boost::asio::ip::address>& data)
    {
        std::vector<boost::optional<boost::asio::ip::address> > opt_inet_addr;
        opt_inet_addr.reserve(data.size());
        BOOST_FOREACH(const boost::asio::ip::address& ip_addr, data)
        {
            opt_inet_addr.push_back(boost::optional<boost::asio::ip::address>(ip_addr));
        }
        return opt_inet_addr;
    }


    std::vector<Fred::DnsHost> make_fred_dns_hosts(const std::vector<DnsHostInput>& data)
    {
        std::vector<Fred::DnsHost> dns_hosts;
        dns_hosts.reserve(data.size());
        BOOST_FOREACH(const DnsHostInput& host, data)
        {
            dns_hosts.push_back(Fred::DnsHost(host.fqdn, make_ipaddrs(host.inet_addr)));
        }
        return dns_hosts;
    }

    std::vector<DnsHostOutput> make_epp_dnshosts_output(const std::vector<Fred::DnsHost>& data)
    {
        std::vector<DnsHostOutput> ret;
        ret.reserve(data.size());
        BOOST_FOREACH(const Fred::DnsHost& dnshost, data)
        {
            ret.push_back(DnsHostOutput(dnshost.get_fqdn(),
                dnshost.get_inet_addr()));
        }
        return ret;
    }

} // namespace Epp::Nsset
} // namespace Epp
