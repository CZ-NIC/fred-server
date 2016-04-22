#include "src/whois/nameserver_exists.h"
#include "tests/interfaces/whois/fixture_common.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_nameserver_by_fqdn)

struct get_nameserver_by_fqdn_fixture
: whois_impl_instance_fixture
{
    std::string test_nameserver_fqdn;

    get_nameserver_by_fqdn_fixture()
    : test_nameserver_fqdn("test_nameserver")
    {
        Fred::OperationContextCreator ctx;
        Fred::InfoRegistrarData registrar;
        Fred::InfoContactData contact;
        Fred::InfoNssetData nsset;

        registrar = Test::registrar::make(ctx);     
        contact = Test::contact::make(ctx);
        nsset = Test::exec(
                    Test::CreateX_factory<Fred::CreateNsset>().make(registrar.handle)
                        //making nameserver
                        .set_dns_hosts(Util::vector_of<Fred::DnsHost>(Fred::DnsHost(test_nameserver_fqdn, Util::vector_of<boost::asio::ip::address>(boost::asio::ip::address()))))
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
     * ip_addresses not tested. for more info refer to implementation
     */
}

BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn_no_ns, whois_impl_instance_fixture)
{
    try
    {
        Registry::WhoisImpl::NameServer ns = impl.get_nameserver_by_fqdn("fine-fqdn.cz");
        BOOST_ERROR("unreported dangling nameserver");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn_wrong_ns, whois_impl_instance_fixture)
{
    try
    {
        Registry::WhoisImpl::NameServer ns = impl.get_nameserver_by_fqdn("");
        BOOST_ERROR("domain handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_nameserver_by_fqdn
BOOST_AUTO_TEST_SUITE_END()//TestWhois
