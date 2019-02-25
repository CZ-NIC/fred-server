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

#include "src/backend/whois/nameserver_exists.hh"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_nameserver_by_fqdn)

struct get_nameserver_by_fqdn_fixture
: whois_impl_instance_fixture
{
    const std::string test_nameserver_fqdn;

    get_nameserver_by_fqdn_fixture()
    : test_nameserver_fqdn("test-nameserver.cz")
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::InfoRegistrarData registrar = Test::registrar::make(ctx);
        const LibFred::InfoContactData contact     = Test::contact::make(ctx);
        Test::exec(
            Test::CreateX_factory<::LibFred::CreateNsset>()
                .make(registrar.handle)
                .set_dns_hosts(  //making nameserver
                    Util::vector_of<::LibFred::DnsHost>(
                        LibFred::DnsHost(
                            test_nameserver_fqdn,
                            Util::vector_of<boost::asio::ip::address>(
                                boost::asio::ip::address::from_string("192.128.0.1")))))
                .set_tech_contacts(Util::vector_of<std::string>(contact.handle)),
            ctx);
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn, get_nameserver_by_fqdn_fixture)
{
    LibFred::OperationContextCreator ctx;
    BOOST_REQUIRE(Fred::Backend::Whois::nameserver_exists(test_nameserver_fqdn, ctx));

    Fred::Backend::Whois::NameServer ns = impl.get_nameserver_by_fqdn(test_nameserver_fqdn);
    BOOST_CHECK(ns.fqdn == test_nameserver_fqdn);
    /*
     * ip_addresses are not tested as they are not added to nameserver
     */
}

BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn_root_dot_query, get_nameserver_by_fqdn_fixture)
{
    LibFred::OperationContextCreator ctx;
    BOOST_REQUIRE(Fred::Backend::Whois::nameserver_exists(test_nameserver_fqdn, ctx));

    Fred::Backend::Whois::NameServer ns = impl.get_nameserver_by_fqdn(test_nameserver_fqdn + ".");
    BOOST_CHECK(ns.fqdn == test_nameserver_fqdn);
    /*
     * ip_addresses are not tested as they are not added to nameserver
     */
}


BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn_no_ns, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_nameserver_by_fqdn("fine-fqdn.cz"), Fred::Backend::Whois::ObjectNotExists);
}

BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn_wrong_ns, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_nameserver_by_fqdn(""), Fred::Backend::Whois::InvalidHandle);
}

BOOST_AUTO_TEST_SUITE_END()//get_nameserver_by_fqdn
BOOST_AUTO_TEST_SUITE_END()//TestWhois
