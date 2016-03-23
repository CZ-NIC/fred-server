//many registrars!
//many registrants!
//contacts could be empty
//keysets!
BOOST_AUTO_TEST_SUITE(get_domains_by_keyset)

struct domains_by_keyset_fixture
: test_registrar_fixture, test_registrant_fixture, test_contact_fixture
{
    std::string test_fqdn;
    std::string test_keyset;
    std::string no_keyset;
    std::string wrong_keyset;
    int regular_domains;

    domains_by_keyset_fixture()
    : test_registrar_fixture(),
      test_registrant_fixture(),
      test_contact_fixture(),
      test_fqdn(std::string("test") + xmark),
      test_keyset("test-nsset" + xmark),
      no_keyset("absent-nsset"),
      wrong_keyset(""),
      regular_domains(6)
    {
        Fred::OperationContext ctx;
        for(int i=0; i < regular_domains - 1; ++i)
        {
            Fred::CreateDomain(test_fqdn + boost::lexical_cast<std::string>(i) + ".cz", test_registrar_handle, test_registrant_handle)
                .set_keyset(test_keyset)
                .set_admin_contacts(Util::vector_of<std::string>(test_admin))
                .exec(ctx);
        }
        for(int i=0; i < 3; ++i)//3 different domains for another keyset
        {
            Fred::CreateDomain(test_fqdn + boost::lexical_cast<std::string>(i) + ".cz", test_registrar_handle, test_registrant_handle)
                .set_keyset(std::string("different-keyset"))
                .set_admin_contacts(Util::vector_of<std::string>("different admin"))
                .exec(ctx);

        }
        //1 with no keyset
        Fred::CreateDomain(test_fqdn + ".cz", test_registrar_handle, test_registrant_handle)
                .set_admin_contacts(Util::vector_of<std::string>(test_admin))
                .exec(ctx);
        Fred::CreateDomain(test_fqdn + ".cz", test_registrar_handle, test_registrant_handle)
                .set_keyset(test_keyset)
                .set_admin_contacts(Util::vector_of<std::string>(test_admin))
                .exec(ctx);
        ctx.commit_transaction();
        BOOST_MESSAGE(test_fqdn);
    }
};

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset, domains_by_keyset_fixture)
{
    Fred::OperationContext ctx;
    const std::vector<Fred::InfoDomainOutput> domain_info =
        Fred::InfoDomainByKeysetHandle(test_keyset)
            .set_limit(regular_domains + 1)
            .exec(ctx, impl.output_timezone);
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_keyset(test_keyset, regular_domains);
    BOOST_CHECK(!domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() == static_cast<unsigned>(regular_domains));
    std::vector<Fred::InfoDomainOutput>::const_iterator found = domain_info.begin(), end = domain_info.end();
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin(); it < domain_vec.end(); ++it)
    {
        while(found != end)
        {
            if(it->fqdn == found->info_domain_data.fqdn) break;
            ++found;
        }
        BOOST_REQUIRE(it->fqdn == found->info_domain_data.fqdn);
        BOOST_CHECK(it->admin_contact_handles.at(0) == found->info_domain_data.admin_contacts.at(0).handle);
        BOOST_CHECK(it->changed.isnull());
        BOOST_CHECK(it->last_transfer.isnull());
        BOOST_CHECK(it->registered == found->info_domain_data.creation_time);
        BOOST_CHECK(it->registrant_handle == found->info_domain_data.registrant.handle);
        BOOST_CHECK(it->registrar_handle == found->info_domain_data.create_registrar_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset_limit_exceeded, domains_by_keyset_fixture)
{
    Fred::OperationContext ctx;
    const std::vector<Fred::InfoDomainOutput> domain_info =
        Fred::InfoDomainByKeysetHandle(test_keyset)
            .set_limit(regular_domains + 1)
            .exec(ctx, impl.output_timezone);
    Registry::WhoisImpl::DomainSeq domain_seq = impl.get_domains_by_keyset(test_keyset, regular_domains);
    BOOST_CHECK(domain_seq.limit_exceeded);

    std::vector<Registry::WhoisImpl::Domain> domain_vec = domain_seq.content;
    BOOST_CHECK(domain_vec.size() != static_cast<unsigned>(regular_domains));
    std::vector<Fred::InfoDomainOutput>::const_iterator found = domain_info.begin(), end = domain_info.end();
    for(std::vector<Registry::WhoisImpl::Domain>::iterator it = domain_vec.begin(); it < domain_vec.end(); ++it)
    {
        while(found != end)
        {
            if(it->fqdn == found->info_domain_data.fqdn) break;
            ++found;
        }
        BOOST_REQUIRE(it->fqdn == found->info_domain_data.fqdn);
        BOOST_CHECK(it->admin_contact_handles.at(0) == found->info_domain_data.admin_contacts.at(0).handle);
        BOOST_CHECK(it->changed.isnull());
        BOOST_CHECK(it->last_transfer.isnull());
        BOOST_CHECK(it->registered == found->info_domain_data.creation_time);
        BOOST_CHECK(it->registrant_handle == found->info_domain_data.registrant.handle);
        BOOST_CHECK(it->registrar_handle == found->info_domain_data.create_registrar_handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset_no_keyset, domains_by_keyset_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_keyset(no_keyset, 0);
        BOOST_ERROR("unreported dangling nsset");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_domains_by_keyset_wrong_keyset, domains_by_keyset_fixture)
{
    try
    {
        Registry::WhoisImpl::DomainSeq ds = impl.get_domains_by_keyset(wrong_keyset, 0);
        BOOST_ERROR("nsset handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//get_domains_by_nsset
