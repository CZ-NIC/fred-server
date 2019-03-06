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
BOOST_AUTO_TEST_SUITE(get_nssets_by_tech_c)

struct get_nssets_by_tech_c_fixture
: whois_impl_instance_fixture
{
    const unsigned int test_limit;
    unsigned int nsset_id;
    boost::posix_time::ptime now_utc;
    std::map<std::string, ::LibFred::InfoNssetData> nsset_info;
    std::string contact_handle;

    get_nssets_by_tech_c_fixture()
    : test_limit(10)
    {
        ::LibFred::OperationContextCreator ctx;
        const ::LibFred::InfoRegistrarData registrar = Test::registrar::make(ctx);
        const ::LibFred::InfoContactData contact     = Test::contact::make(ctx);
        const ::LibFred::InfoNssetData nsset         = Test::nsset::make(ctx);
        contact_handle = contact.handle;
        now_utc = boost::posix_time::time_from_string(
                      static_cast<std::string>(
                          ctx.get_conn().exec("SELECT now()::timestamp")[0][0]));
        nsset_id = nsset.id;
        Util::vector_of<::LibFred::DnsHost> dns_hosts(::LibFred::DnsHost(
            "some-fqdn",
            Util::vector_of<boost::asio::ip::address>(
                boost::asio::ip::address::from_string("192.128.0.1"))));
        for(unsigned int i = 0; i < test_limit; ++i)
        {
            const ::LibFred::InfoNssetData& ind = Test::exec(
                    Test::CreateX_factory<::LibFred::CreateNsset>()
                        .make(registrar.handle)
                        .set_dns_hosts(dns_hosts)
                        .set_tech_contacts(Util::vector_of<std::string>(contact.handle)),
                    ctx);
            nsset_info[ind.handle] = ind;
        }
        //different contact's nssets
        for(unsigned int i = 0; i < 3; ++i)
        {
            Test::exec(
                Test::CreateX_factory<::LibFred::CreateNsset>()
                    .make(registrar.handle)
                    .set_dns_hosts(dns_hosts)
                    .set_tech_contacts(
                        Util::vector_of<std::string>(
                            Test::contact::make(ctx).handle)),
                ctx);
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_nssets_by_tech_c, get_nssets_by_tech_c_fixture)
{
    Fred::Backend::Whois::NSSetSeq nss_s = impl.get_nssets_by_tech_c(contact_handle, test_limit);

    BOOST_CHECK(!nss_s.limit_exceeded);
    BOOST_CHECK(nss_s.content.size() == test_limit);
    std::map<std::string, ::LibFred::InfoNssetData>::const_iterator cit;
    BOOST_FOREACH(const Fred::Backend::Whois::NSSet& it, nss_s.content)
    {
        cit = nsset_info.find(it.handle);
        BOOST_REQUIRE(cit != nsset_info.end());
        const ::LibFred::InfoNssetData& found = cit->second;
        BOOST_REQUIRE(it.handle                          == found.handle);
        BOOST_CHECK(it.created                           == now_utc);
        BOOST_CHECK(it.handle                            == found.handle);
        BOOST_CHECK(it.nservers.at(0).fqdn               == found.dns_hosts.at(0).get_fqdn());
        BOOST_CHECK(it.nservers.at(0).ip_addresses.at(0) == found.dns_hosts.at(0).get_inet_addr().at(0));
        BOOST_CHECK(it.sponsoring_registrar              == found.sponsoring_registrar_handle);
        BOOST_CHECK(it.tech_contacts.at(0)               == found.tech_contacts.at(0).handle);
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.last_transfer.isnull());

        ::LibFred::OperationContextCreator ctx;
        const std::vector<::LibFred::ObjectStateData> v_osd = ::LibFred::GetObjectStates(nsset_id).exec(ctx);
        BOOST_FOREACH(const ::LibFred::ObjectStateData& oit, v_osd)
        {
            BOOST_CHECK(std::find(it.statuses.begin(), it.statuses.end(), oit.state_name) !=
                            it.statuses.end());
        }
        BOOST_CHECK(it.statuses.size() == v_osd.size());
    }
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_tech_c_limit_exceeded, get_nssets_by_tech_c_fixture)
{
    Fred::Backend::Whois::NSSetSeq nss_s = impl.get_nssets_by_tech_c(contact_handle, test_limit - 1);

    BOOST_CHECK(nss_s.limit_exceeded);
    BOOST_CHECK(nss_s.content.size() == test_limit - 1);
    std::map<std::string, ::LibFred::InfoNssetData>::const_iterator cit;
    BOOST_FOREACH(const Fred::Backend::Whois::NSSet& it, nss_s.content)
    {
        cit = nsset_info.find(it.handle);
        BOOST_REQUIRE(cit != nsset_info.end());
        const ::LibFred::InfoNssetData& found = cit->second;
        BOOST_REQUIRE(it.handle                          == found.handle);
        BOOST_CHECK(it.created                           == now_utc);
        BOOST_CHECK(it.handle                            == found.handle);
        BOOST_CHECK(it.nservers.at(0).fqdn               == found.dns_hosts.at(0).get_fqdn());
        BOOST_CHECK(it.nservers.at(0).ip_addresses.at(0) == found.dns_hosts.at(0).get_inet_addr().at(0));
        BOOST_CHECK(it.sponsoring_registrar              == found.sponsoring_registrar_handle);
        BOOST_CHECK(it.tech_contacts.at(0)               == found.tech_contacts.at(0).handle);
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.last_transfer.isnull());

        ::LibFred::OperationContextCreator ctx;
        const std::vector<::LibFred::ObjectStateData> v_osd = ::LibFred::GetObjectStates(nsset_id).exec(ctx);
        BOOST_FOREACH(const ::LibFred::ObjectStateData& oit, v_osd)
        {
            BOOST_CHECK(std::find(it.statuses.begin(), it.statuses.end(), oit.state_name) !=
                            it.statuses.end());
        }
        BOOST_CHECK(it.statuses.size() == v_osd.size());
    }
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_tech_c_no_ns, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_nssets_by_tech_c("absent-contact", 1), Fred::Backend::Whois::ObjectNotExists);
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_tech_c_wrong_ns, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_nssets_by_tech_c("", 1), Fred::Backend::Whois::InvalidHandle);
}

BOOST_AUTO_TEST_SUITE_END()//get_nssets_by_tech_c
BOOST_AUTO_TEST_SUITE_END()//TestWhois
