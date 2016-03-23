//registrar!
//contact could be empty
BOOST_AUTO_TEST_SUITE(get_nameserver_by_fqdn)

struct get_nameserver_by_fqdn_fixture
: test_registrar_fixture
{
    std::string test_nameserver_fqdn;
    std::string test_no_handle;
    std::string test_wrong_handle;

    get_nameserver_by_fqdn_fixture()
    : test_registrar_fixture(),
      test_nameserver_fqdn(std::string("test-nameserver") + xmark + ".cz"),
      test_no_handle("fine-fqdn.cz"),
      test_wrong_handle("")
    {
        Fred::OperationContext ctx;
        Fred::CreateNsset("TEST-NSSET-HANDLE", test_registrar_handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>(Fred::DnsHost(test_nameserver_fqdn, Util::vector_of<boost::asio::ip::address>(boost::asio::ip::address()))))//making nameserver
            .set_tech_contacts(Util::vector_of<std::string>("TEST-TECH-CONTACT"))
            .exec(ctx);
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn, get_nameserver_by_fqdn_fixture)
{
    Fred::OperationContext ctx;
    BOOST_REQUIRE(Whois::nameserver_exists(test_nameserver_fqdn, ctx));

    Registry::WhoisImpl::NameServer ns = impl.get_nameserver_by_fqdn(test_nameserver_fqdn);
    BOOST_CHECK(ns.fqdn == test_nameserver_fqdn);
}

BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn_no_ns, get_nameserver_by_fqdn_fixture)
{
    try
    {
        Registry::WhoisImpl::NameServer ns = impl.get_nameserver_by_fqdn(test_no_handle);
        BOOST_ERROR("unreported dangling nameserver");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_nameserver_by_fqdn_wrong_ns, get_nameserver_by_fqdn_fixture)
{
    try
    {
        Registry::WhoisImpl::NameServer ns = impl.get_nameserver_by_fqdn(test_wrong_handle);
        BOOST_ERROR("domain handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_nameserver_by_fqdn
