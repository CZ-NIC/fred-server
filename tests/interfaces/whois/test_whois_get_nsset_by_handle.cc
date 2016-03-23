//registrar!
BOOST_AUTO_TEST_SUITE(get_nsset_by_handle)

struct get_nsset_by_handle_fixture
: test_registrar_fixture
{
    std::string test_nsset_handle;
    std::string no_nsset_handle;
    std::string wrong_nsset_handle;

    get_nsset_by_handle_fixture()
    : test_registrar_fixture(),
      test_nsset_handle(std::string("TEST-NSSET-HANDLE")+xmark),
      no_nsset_handle("fine-nsset-handle"),
      wrong_nsset_handle("")
    {
        Fred::OperationContext ctx;
        std::vector<std::string> tech_contacts;
        tech_contacts.push_back("TEST-TECH-CONTACT");

        Fred::CreateNsset(test_nsset_handle, test_registrar_handle)
            .set_dns_hosts(Util::vector_of<Fred::DnsHost>(
                    Fred::DnsHost(std::string("TEST-FQDN")+xmark,
                    Util::vector_of<boost::asio::ip::address>(boost::asio::ip::address()))))
            .set_tech_contacts(Util::vector_of<std::string>("TEST-TECH-CONTACT"))//could be empty
            .exec(ctx);
        ctx.commit_transaction();
        BOOST_MESSAGE(test_nsset_handle);
    }
};

BOOST_FIXTURE_TEST_CASE(get_nsset_by_handle, get_nsset_by_handle_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoNssetData ind = Fred::InfoNssetByHandle(test_nsset_handle).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone).info_nsset_data;
    Registry::WhoisImpl::NSSet nss = impl.get_nsset_by_handle(test_nsset_handle);

    BOOST_CHECK(nss.changed.isnull());//new nsset has to be unchanged
    BOOST_CHECK(nss.last_transfer.isnull());//new nsset has to not transferred
    BOOST_CHECK(nss.created == ind.creation_time);//as that or greater than __
    BOOST_CHECK(nss.handle == ind.handle);
    BOOST_CHECK(nss.nservers.at(0).fqdn == ind.dns_hosts.at(0).get_fqdn());
    BOOST_CHECK(nss.nservers.at(0).ip_addresses.at(0) == ind.dns_hosts.at(0).get_inet_addr().at(0)); //comparing two boost::address'es
    BOOST_CHECK(nss.registrar_handle == ind.create_registrar_handle);
    BOOST_CHECK(nss.tech_contact_handles.at(0) == ind.tech_contacts.at(0).handle);
}

BOOST_FIXTURE_TEST_CASE(get_nsset_by_handle_no_nsset, get_nsset_by_handle_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSet nss = impl.get_nsset_by_handle(no_nsset_handle);
        BOOST_ERROR("unreported dangling nsset");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_nsset_by_handle_wrong_nsset, get_nsset_by_handle_fixture)
{
    try
    {
        Registry::WhoisImpl::NSSet nss = impl.get_nsset_by_handle(wrong_nsset_handle);
        BOOST_ERROR("nsset handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}


BOOST_AUTO_TEST_SUITE_END()//get_nsset_by_handle
