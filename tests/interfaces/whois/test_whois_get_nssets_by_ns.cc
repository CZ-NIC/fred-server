//many registrars!
BOOST_AUTO_TEST_SUITE(get_nssets_by_ns)

struct get_nssets_by_ns_fixture
: test_registrar_fixture
{
    std::string test_fqdn;
    std::string test_no_fqdn;
    std::string test_wrong_fqdn;
    unsigned long test_limit;

    get_nssets_by_ns_fixture()
    : test_registrar_fixture(),
      test_fqdn(std::string("test") + xmark + ".cz"),
      test_no_fqdn("fine-fqdn.cz"),
      test_wrong_fqdn("."),
      test_limit(10)
    {
        Fred::OperationContext ctx;
        for(unsigned long i = 0; i < test_limit; ++i)
        {
            std::ostringstream test_handles;
            test_handles << "n" << i;
            std::vector<Fred::DnsHost> v_dns;
            v_dns.push_back(Fred::DnsHost(test_fqdn,
                            Util::vector_of<boost::asio::ip::address>(boost::asio::ip::address())));
            std::vector<std::string> tech_contacts;
            tech_contacts.push_back("TEST-TECH-CONTACT" + xmark);

            Fred::CreateNsset(test_handles.str(), test_registrar_handle + boost::lexical_cast<std::string>(i),
                              Optional<std::string>(), Optional<short>(), v_dns,
                              tech_contacts, //could be empty
                              Optional<unsigned long long>())
                .exec(ctx);
            //Jiri: any extra setting here?
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_nssets_by_ns, get_nssets_by_ns_fixture)
{
    Fred::OperationContext ctx;
    std::vector<Fred::InfoNssetOutput> v_ino = Fred::InfoNssetByDNSFqdn(test_fqdn).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone);
    Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_ns(test_fqdn, test_limit);
    for(unsigned long i = 0; i < test_limit; ++i)
    {
        std::vector<Fred::InfoNssetOutput>::iterator it = v_ino.begin(), end = v_ino.end();
        while(it != end)
        {
            if(it->info_nsset_data.handle == nss_s.content.at(i).handle) break;
            ++it;
        }
        BOOST_REQUIRE(nss_s.content.at(i).handle == it->info_nsset_data.handle);
        Fred::InfoNssetData found = it->info_nsset_data;

        BOOST_CHECK(nss_s.content.at(i).changed.isnull());
        BOOST_CHECK(nss_s.content.at(i).last_transfer.isnull());
        BOOST_CHECK(nss_s.content.at(i).created == found.creation_time);//as that or greater than __
        BOOST_CHECK(nss_s.content.at(i).handle == found.handle);
        BOOST_CHECK(nss_s.content.at(i).nservers.at(0).fqdn == found.dns_hosts.at(0).get_fqdn());
        BOOST_CHECK(nss_s.content.at(i).nservers.at(0).ip_addresses.at(0) == found.dns_hosts.at(0).get_inet_addr().at(0)); //comparing two boost::address'es
        BOOST_CHECK(nss_s.content.at(i).registrar_handle == found.create_registrar_handle);
        BOOST_CHECK(nss_s.content.at(i).tech_contact_handles.at(0) == found.tech_contacts.at(0).handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_ns_no_ns, get_nssets_by_ns_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_ns(test_no_fqdn, test_limit);
        BOOST_ERROR("unreported dangling NSSets");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_ns_wrong_ns, get_nssets_by_ns_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_ns(test_wrong_fqdn, test_limit);
        BOOST_ERROR("domain handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_nssets_by_ns
