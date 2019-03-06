/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#include "test/backend/whois/fixture_common.hh"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_nsset_by_handle)

struct get_nsset_by_handle_fixture
: whois_impl_instance_fixture
{
    const std::string test_nsset_handle;
    unsigned long long id;
    boost::posix_time::ptime now_utc;
    LibFred::InfoNssetData nsset;

    get_nsset_by_handle_fixture()
    : test_nsset_handle("TEST-NSSET")
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::InfoContactData contact = Test::contact::make(ctx);
        nsset = Test::exec(
                    Test::CreateX_factory<::LibFred::CreateNsset>()
                        .make(Test::registrar::make(ctx).handle, test_nsset_handle)
                        .set_dns_hosts(
                            Util::vector_of<::LibFred::DnsHost>(
                                LibFred::DnsHost(
                                    "some-nameserver-1.cz",
                                    Util::vector_of<boost::asio::ip::address>(
                                        boost::asio::ip::address::from_string("192.128.0.1"))))(
                                LibFred::DnsHost(
                                    "some-nameserver-2.cz",
                                    Util::vector_of<boost::asio::ip::address>(
                                        boost::asio::ip::address::from_string("192.128.0.21"))(
                                        boost::asio::ip::address::from_string("192.128.0.22"))(
                                        boost::asio::ip::address::from_string("2001::20")))))
                        .set_tech_contacts(
                            Util::vector_of<std::string>(contact.handle)),
                        ctx);
        now_utc = boost::posix_time::time_from_string(
                    static_cast<std::string>(ctx.get_conn()
                        .exec("SELECT now()::timestamp")[0][0]));
        id = nsset.id;
        ctx.commit_transaction();
    }
};

namespace {

bool is_member(const std::string &_fqdn, const std::vector< LibFred::DnsHost > &_hosts)
{
    for (::size_t idx = 0; idx < _hosts.size(); ++idx)
    {
        if (_hosts[idx].get_fqdn() == _fqdn)
        {
            return true;
        }
    }
    return false;
}

const LibFred::DnsHost& get_member(const std::string &_fqdn, const std::vector< LibFred::DnsHost > &_hosts)
{
    for (::size_t idx = 0; idx < _hosts.size(); ++idx)
    {
        if (_hosts[idx].get_fqdn() == _fqdn)
        {
            return _hosts[idx];
        }
    }
    throw std::runtime_error("DnsHost not found");
}

bool is_member(const boost::asio::ip::address &_address, const std::vector< boost::asio::ip::address > &_addresses)
{
    for (::size_t idx = 0; idx < _addresses.size(); ++idx)
    {
        if (_addresses[idx] == _address)
        {
            return true;
        }
    }
    return false;
}

} // namespace {anonymous}

BOOST_FIXTURE_TEST_CASE(get_nsset_by_handle, get_nsset_by_handle_fixture)
{
    Fred::Backend::Whois::NSSet nss = impl.get_nsset_by_handle(test_nsset_handle);

    BOOST_CHECK(nss.changed.isnull());
    BOOST_CHECK(nss.last_transfer.isnull());
    BOOST_CHECK(nss.created == now_utc);
    BOOST_CHECK(nss.handle == nsset.handle);
    BOOST_CHECK(nss.nservers.size() == nsset.dns_hosts.size());
    for (::size_t nserver_idx = 0; nserver_idx < nss.nservers.size(); ++nserver_idx)
    {
        const bool has_dns_host = is_member(nss.nservers[nserver_idx].fqdn, nsset.dns_hosts);
        BOOST_CHECK(has_dns_host);
        if (has_dns_host)
        {
            const LibFred::DnsHost &dns_host = get_member(nss.nservers[nserver_idx].fqdn, nsset.dns_hosts);
            BOOST_CHECK(nss.nservers[nserver_idx].ip_addresses.size() == dns_host.get_inet_addr().size());
            for (::size_t addr_idx = 0; addr_idx < nss.nservers[nserver_idx].ip_addresses.size(); ++addr_idx)
            {
                BOOST_CHECK(is_member(nss.nservers[nserver_idx].ip_addresses[addr_idx], dns_host.get_inet_addr()));
            }
        }
    }
    BOOST_CHECK(nss.sponsoring_registrar == nsset.sponsoring_registrar_handle);
    BOOST_CHECK(nss.tech_contacts.at(0) == nsset.tech_contacts.at(0).handle);

    LibFred::OperationContextCreator ctx;
    const std::vector<::LibFred::ObjectStateData> v_osd = LibFred::GetObjectStates(id).exec(ctx);
    BOOST_FOREACH(const LibFred::ObjectStateData& it, v_osd)
    {
        BOOST_CHECK(std::find(nss.statuses.begin(), nss.statuses.end(), it.state_name) !=
                nss.statuses.end());
    }
    BOOST_CHECK(nss.statuses.size() == v_osd.size());
}

BOOST_FIXTURE_TEST_CASE(get_nsset_by_handle_no_nsset, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_nsset_by_handle("fine-nsset-handle"), Fred::Backend::Whois::ObjectNotExists);
}

BOOST_FIXTURE_TEST_CASE(get_nsset_by_handle_wrong_nsset, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_nsset_by_handle(""), Fred::Backend::Whois::InvalidHandle);
}

BOOST_AUTO_TEST_SUITE_END()//get_nsset_by_handle
BOOST_AUTO_TEST_SUITE_END()//TestWhois
