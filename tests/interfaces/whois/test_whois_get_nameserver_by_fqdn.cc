#include "tests/interfaces/whois/fixture_common.h"

#include "src/whois/nameserver_exists.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_nameserver_by_fqdn)

struct get_nameserver_by_fqdn_fixture
: whois_impl_instance_fixture
{
    const std::string test_nameserver_fqdn;

    get_nameserver_by_fqdn_fixture()
    : test_nameserver_fqdn("test-nameserver.cz")
    {
        Fred::OperationContextCreator ctx;
        const Fred::InfoRegistrarData registrar = Test::registrar::make(ctx);
        const Fred::InfoContactData contact     = Test::contact::make(ctx);
        Test::exec(
            Test::CreateX_factory<Fred::CreateNsset>()
                .make(registrar.handle)
                .set_dns_hosts(  //making nameserver
                    Util::vector_of<Fred::DnsHost>(
                        Fred::DnsHost(
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
    Fred::OperationContextCreator ctx;
    BOOST_REQUIRE(Whois::nameserver_exists(test_nameserver_fqdn, ctx));

    Registry::WhoisImpl::NameServer ns = impl.get_nameserver_by_fqdn(test_nameserver_fqdn);
    BOOST_CHECK(ns.fqdn == test_nameserver_fqdn);
    /*
     * ip_addresses are not tested as they are not added to nameserver
     */
}

BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn_root_dot_query, get_nameserver_by_fqdn_fixture)
{
    Fred::OperationContextCreator ctx;
    BOOST_REQUIRE(Whois::nameserver_exists(test_nameserver_fqdn, ctx));

    Registry::WhoisImpl::NameServer ns = impl.get_nameserver_by_fqdn(test_nameserver_fqdn + ".");
    BOOST_CHECK(ns.fqdn == test_nameserver_fqdn);
    /*
     * ip_addresses are not tested as they are not added to nameserver
     */
}


BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn_no_ns, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_nameserver_by_fqdn("fine-fqdn.cz"), Registry::WhoisImpl::ObjectNotExists);
}

BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn_wrong_ns, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_nameserver_by_fqdn(""), Registry::WhoisImpl::InvalidHandle);
}

BOOST_AUTO_TEST_SUITE_END()//get_nameserver_by_fqdn
BOOST_AUTO_TEST_SUITE_END()//TestWhois
