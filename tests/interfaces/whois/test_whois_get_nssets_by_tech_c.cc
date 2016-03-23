//many registrars!
//many contacts!
BOOST_AUTO_TEST_SUITE(get_nssets_by_tech_c)

struct get_nssets_by_tech_c_fixture
: test_registrar_fixture
{
    std::string test_contact_handle;
    std::string test_no_handle;//better name
    std::string test_wrong_handle;
    unsigned long test_limit;

    get_nssets_by_tech_c_fixture()
    : test_registrar_fixture(),
      test_contact_handle(std::string("TEST-CONTACT-HANDLE") + xmark),
      test_no_handle("fine-tech-c-handle"),
      test_wrong_handle(""),
      test_limit(10)
    {
        Fred::OperationContext ctx;
        for(unsigned long i = 0; i < test_limit; ++i)
        {
            std::ostringstream test_handles;
            test_handles << "n" << i;
            std::vector<Fred::DnsHost> v_dns;
            v_dns.push_back(Fred::DnsHost(test_contact_handle,
                            Util::vector_of<boost::asio::ip::address>(boost::asio::ip::address())));
            std::vector<std::string> tech_contacts;
            tech_contacts.push_back("TEST-TECH-CONTACT" + xmark);

            Fred::CreateNsset(test_handles.str(), test_registrar_handle + boost::lexical_cast<std::string>(i),
                              Optional<std::string>(), Optional<short>(), v_dns,
                              tech_contacts, Optional<unsigned long long>())
                .exec(ctx);
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_nssets_by_tech_c, get_nssets_by_tech_c_fixture)
{
    Fred::OperationContext ctx;
    std::vector<Fred::InfoNssetOutput> v_ino = Fred::InfoNssetByDNSFqdn(test_contact_handle).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone);
    Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_tech_c(test_contact_handle, test_limit);
    for(unsigned long i = 0; i < test_limit; ++i)
    {
        std::vector<Fred::InfoNssetOutput>::iterator it = v_ino.begin(), end = v_ino.end();
        while(it != end)
        {
            if(it->info_nsset_data.handle == nss_s.content.at(i).handle) break;
            ++it;
        }
        BOOST_CHECK(nss_s.content.at(i).handle == it->info_nsset_data.handle);
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

BOOST_FIXTURE_TEST_CASE(get_nssets_by_tech_c_no_ns, get_nssets_by_tech_c_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_tech_c(test_no_handle, test_limit);
        BOOST_ERROR("unreported dangling NSSets");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_nssets_by_tech_c_wrong_ns, get_nssets_by_tech_c_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSetSeq nss_s = impl.get_nssets_by_tech_c(test_wrong_handle, test_limit);
        BOOST_ERROR("domain handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_nssets_by_tech_c
