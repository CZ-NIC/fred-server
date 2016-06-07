#include "tests/interfaces/whois/fixture_common.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_nssets_by_ns)

struct get_nssets_by_ns_fixture
: whois_impl_instance_fixture
{
    typedef Registry::WhoisImpl::NSSet NSSet;

    std::string test_fqdn;
    unsigned int test_limit;
    Fred::InfoRegistrarData registrar;
    Fred::InfoContactData contact;
    Fred::InfoNssetData nsset;
    boost::posix_time::ptime now_utc;
    std::map<std::string, Fred::InfoNssetData> nsset_info;

    get_nssets_by_ns_fixture()
    : test_fqdn("test-fqdn"),
      test_limit(10)
    {
        Fred::OperationContextCreator ctx;
        registrar = Test::registrar::make(ctx);
        contact = Test::contact::make(ctx);
        now_utc = boost::posix_time::time_from_string(
                static_cast<std::string>(ctx.get_conn()
                    .exec("SELECT now()::timestamp")[0][0]));
        for(unsigned int i = 0; i < test_limit; ++i)
        {
            Util::vector_of<Fred::DnsHost> dns_hosts(
                Fred::DnsHost(test_fqdn,
                              Util::vector_of<boost::asio::ip::address>(
                                  boost::asio::ip::address())));
            const Fred::InfoNssetData& ind = Test::exec(
                Test::CreateX_factory<Fred::CreateNsset>()
                    .make(registrar.handle)
                    .set_dns_hosts(dns_hosts)
                    .set_tech_contacts(
                        Util::vector_of<std::string>(contact.handle)),
                ctx);
            nsset_info[ind.handle] = ind;
        }
        //different NS nssets
        for(unsigned int i = 0; i < 3; ++i)
        {
            Util::vector_of<Fred::DnsHost> other_hosts(
                Fred::DnsHost("other-fqdn", 
                              Util::vector_of<boost::asio::ip::address>(
                                  boost::asio::ip::address())));
            Test::exec(
                Test::CreateX_factory<Fred::CreateNsset>()
                    .make(registrar.handle)
                    .set_dns_hosts(other_hosts)
                    .set_tech_contacts(
                        Util::vector_of<std::string>(contact.handle)),
                ctx);
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_nssets_by_ns, get_nssets_by_ns_fixture)
{
    Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_ns(test_fqdn,
                                                                test_limit);
    
    BOOST_CHECK(!nss_s.limit_exceeded);
    BOOST_CHECK(nss_s.content.size() == test_limit);
    std::map<std::string, Fred::InfoNssetData>::iterator found;
    BOOST_FOREACH(NSSet it, nss_s.content)
    {
        found = nsset_info.find(it.handle);
        BOOST_REQUIRE(it.handle == found->second.handle);
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.created == now_utc);
        BOOST_CHECK(it.handle == found->second.handle);
        BOOST_CHECK(it.nservers.at(0).fqdn == found->second.dns_hosts.at(0).get_fqdn());
        BOOST_CHECK(it.nservers.at(0).ip_addresses.at(0) == found->second.dns_hosts.at(0).get_inet_addr().at(0));
        BOOST_CHECK(it.registrar == found->second.create_registrar_handle);
        BOOST_CHECK(it.tech_contacts.at(0) == found->second.tech_contacts.at(0).handle);

        Fred::OperationContextCreator ctx;
        const std::vector<Fred::ObjectStateData> v_osd =
            Fred::GetObjectStates(nsset.id).exec(ctx);
        BOOST_FOREACH(const Fred::ObjectStateData oit, v_osd)
        {
            BOOST_CHECK(std::find(it.statuses.begin(), it.statuses.end(), oit.state_name) !=
                            it.statuses.end());
        }
        BOOST_CHECK(it.statuses.size() == v_osd.size());
    }
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_ns_limit_exceeded, get_nssets_by_ns_fixture)
{
    Registry::WhoisImpl::NSSetSeq nss_s =
        impl.get_nssets_by_ns(test_fqdn, test_limit - 1);
    
    BOOST_CHECK(nss_s.limit_exceeded); BOOST_CHECK(nss_s.content.size() == test_limit - 1);
    std::map<std::string, Fred::InfoNssetData>::iterator found;
    BOOST_FOREACH(NSSet it, nss_s.content)
    {
        found = nsset_info.find(it.handle);
        BOOST_REQUIRE(it.handle == found->second.handle);
        BOOST_CHECK(it.changed.isnull());
        BOOST_CHECK(it.last_transfer.isnull());
        BOOST_CHECK(it.created == now_utc);
        BOOST_CHECK(it.handle == found->second.handle);
        BOOST_CHECK(it.nservers.at(0).fqdn == found->second.dns_hosts.at(0).get_fqdn());
        BOOST_CHECK(it.nservers.at(0).ip_addresses.at(0) == found->second.dns_hosts.at(0).get_inet_addr().at(0));
        BOOST_CHECK(it.registrar == found->second.create_registrar_handle);
        BOOST_CHECK(it.tech_contacts.at(0) == found->second.tech_contacts.at(0).handle);

        Fred::OperationContextCreator ctx;
        const std::vector<Fred::ObjectStateData> v_osd =
            Fred::GetObjectStates(nsset.id).exec(ctx);
        BOOST_FOREACH(const Fred::ObjectStateData oit, v_osd)
        {
            BOOST_CHECK(std::find(it.statuses.begin(), it.statuses.end(), oit.state_name) !=
                            it.statuses.end());
        }
        BOOST_CHECK(it.statuses.size() == v_osd.size());
    }
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_ns_no_ns, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(get_nssets_by_ns("fine-fqdn.cz", 1), Registry::WhoisImpl::ObjectNotExists)
//    try
//    {
//        Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_ns("fine-fqdn.cz", 1);
//        BOOST_ERROR("unreported dangling NSSets");
//    }
//    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
//    {
//        BOOST_CHECK(true);
//        BOOST_MESSAGE(boost::diagnostic_information(ex));
//    }
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_ns_wrong_ns, whois_impl_instance_fixture)
{
    BOOST_CHECK_THROW(get_nssets_by_ns(".", 1), Registry::WhoisImpl::InvalidHandle)
//    try
//    {
//        Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_ns(".", 1);
//        BOOST_ERROR("domain handle rule is wrong");
//    }
//    catch(const Registry::WhoisImpl::InvalidHandle& ex)
//    {
//        BOOST_CHECK(true);
//        BOOST_MESSAGE(boost::diagnostic_information(ex));
//    }
}

BOOST_AUTO_TEST_SUITE_END()//get_nssets_by_ns
BOOST_AUTO_TEST_SUITE_END()//TestWhois
