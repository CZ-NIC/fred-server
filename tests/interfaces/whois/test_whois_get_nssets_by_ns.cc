#include "tests/interfaces/whois/fixture_common.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_nssets_by_ns)

struct get_nssets_by_ns_fixture
: whois_impl_instance_fixture
{
    const std::string test_fqdn;
    const unsigned int test_limit;
    unsigned int nsset_id;
    std::map<std::string, Fred::InfoNssetData> nsset_info;
    boost::posix_time::ptime now_utc;

    get_nssets_by_ns_fixture()
    : test_fqdn("test-fqdn"),
      test_limit(10)
    {
        Fred::OperationContextCreator ctx;
        const Fred::InfoRegistrarData registrar = Test::registrar::make(ctx);
        const Fred::InfoContactData contact     = Test::contact::make(ctx);
        const Fred::InfoNssetData nsset         = Test::nsset::make(ctx);
        now_utc = boost::posix_time::time_from_string(
                      static_cast<std::string>(
                          ctx.get_conn().exec("SELECT now()::timestamp")[0][0]));
        nsset_id = nsset.id;
        for(unsigned int i = 0; i < test_limit; ++i)
        {
            const Fred::InfoNssetData& ind = Test::exec(
                    Test::CreateX_factory<Fred::CreateNsset>()
                    .make(registrar.handle)
                    .set_dns_hosts(
                        Util::vector_of<Fred::DnsHost>(
                            Fred::DnsHost(
                                test_fqdn,
                                Util::vector_of<boost::asio::ip::address>(
                                    boost::asio::ip::address::from_string("192.128.0.1")))))
                    .set_tech_contacts(
                            Util::vector_of<std::string>(contact.handle)),
                ctx);
            nsset_info[ind.handle] = ind;
        }
        //different NS nssets
        for(unsigned int i = 0; i < 3; ++i)
        {
            Test::exec(
                Test::CreateX_factory<Fred::CreateNsset>()
                    .make(registrar.handle)
                    .set_dns_hosts(
                        Util::vector_of<Fred::DnsHost>(
                            Fred::DnsHost(
                                "other-fqdn", 
                                Util::vector_of<boost::asio::ip::address>(
                                    boost::asio::ip::address::from_string("192.128.1.1")))))
                    .set_tech_contacts(
                            Util::vector_of<std::string>(contact.handle)),
                ctx);
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_nssets_by_ns, get_nssets_by_ns_fixture)
{
    Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_ns(test_fqdn, test_limit);
    
    BOOST_CHECK(!nss_s.limit_exceeded);
    BOOST_CHECK(nss_s.content.size() == test_limit);
    std::map<std::string, Fred::InfoNssetData>::iterator found;
    BOOST_FOREACH(const Registry::WhoisImpl::NSSet& it, nss_s.content)
    {
        found = nsset_info.find(it.handle);
        BOOST_REQUIRE(it.handle == found->second.handle);
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.created == now_utc);
        BOOST_CHECK(it.handle == found->second.handle);
        BOOST_CHECK(it.nservers.at(0).fqdn == found->second.dns_hosts.at(0).get_fqdn());
        BOOST_CHECK(it.nservers.at(0).ip_addresses.at(0) == found->second.dns_hosts.at(0).get_inet_addr().at(0));
        BOOST_CHECK(it.creating_registrar == found->second.create_registrar_handle);
        BOOST_CHECK(it.tech_contacts.at(0) == found->second.tech_contacts.at(0).handle);

        Fred::OperationContext ctx;
        const std::vector<Fred::ObjectStateData> v_osd = Fred::GetObjectStates(nsset_id).exec(ctx);
        BOOST_FOREACH(const Fred::ObjectStateData& oit, v_osd)
        {
            BOOST_CHECK(std::find(it.statuses.begin(), it.statuses.end(), oit.state_name) !=
                            it.statuses.end());
        }
        BOOST_CHECK(it.statuses.size() == v_osd.size());
    }
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_ns_limit_exceeded, get_nssets_by_ns_fixture)
{
    Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_ns(test_fqdn, test_limit - 1);
    
    BOOST_CHECK(nss_s.limit_exceeded); BOOST_CHECK(nss_s.content.size() == test_limit - 1);
    std::map<std::string, Fred::InfoNssetData>::iterator found;
    BOOST_FOREACH(const Registry::WhoisImpl::NSSet& it, nss_s.content)
    {
        found = nsset_info.find(it.handle);
        BOOST_REQUIRE(it.handle == found->second.handle);
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.created == now_utc);
        BOOST_CHECK(it.handle == found->second.handle);
        BOOST_CHECK(it.nservers.at(0).fqdn == found->second.dns_hosts.at(0).get_fqdn());
        BOOST_CHECK(it.nservers.at(0).ip_addresses.at(0) == found->second.dns_hosts.at(0).get_inet_addr().at(0));
        BOOST_CHECK(it.creating_registrar == found->second.create_registrar_handle);
        BOOST_CHECK(it.tech_contacts.at(0) == found->second.tech_contacts.at(0).handle);

        Fred::OperationContext ctx;
        const std::vector<Fred::ObjectStateData> v_osd = Fred::GetObjectStates(nsset.id).exec(ctx);
        BOOST_FOREACH(const Fred::ObjectStateData& oit, v_osd)
        {
            BOOST_CHECK(std::find(it.statuses.begin(), it.statuses.end(), oit.state_name) !=
                            it.statuses.end());
        }
        BOOST_CHECK(it.statuses.size() == v_osd.size());
    }
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_ns_no_ns, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_nssets_by_ns("fine-fqdn.cz", 1), Registry::WhoisImpl::ObjectNotExists)
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_ns_wrong_ns, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(impl.get_nssets_by_ns(".", 1), Registry::WhoisImpl::InvalidHandle)
}

BOOST_AUTO_TEST_SUITE_END()//get_nssets_by_ns
BOOST_AUTO_TEST_SUITE_END()//TestWhois
