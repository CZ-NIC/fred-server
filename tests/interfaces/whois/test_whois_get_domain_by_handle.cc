//registrar!
//registrant!
//contact could be empty
BOOST_AUTO_TEST_SUITE(get_domain_by_handle)

struct test_domain_fixture
: test_registrar_fixture, test_registrant_fixture, test_contact_fixture
{
    std::string test_fqdn;
    std::string no_fqdn;
    std::string wrong_fqdn;
    std::string invalid_fqdn;

    test_domain_fixture()
    : test_registrar_fixture(),
      test_registrant_fixture(),
      test_contact_fixture(),
      test_fqdn(std::string("test") + xmark + ".cz"),
      no_fqdn("fine-handle.cz"),
      wrong_fqdn(""),
      invalid_fqdn("a-.cz")
    {
        Fred::OperationContext ctx;
        Fred::CreateDomain(test_fqdn, test_registrar_handle, test_registrant_handle)
            .set_admin_contacts(Util::vector_of<std::string>(test_admin))
            .exec(ctx);
        ctx.commit_transaction();
        BOOST_MESSAGE(test_fqdn);
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle, test_domain_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoDomainData idd = Fred::InfoDomainByHandle(test_fqdn).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone).info_domain_data;
    Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(test_fqdn);
    BOOST_CHECK(dom.admin_contact_handles.at(0) == idd.admin_contacts.at(0).handle);
    BOOST_CHECK(dom.changed.isnull());
    BOOST_CHECK(dom.fqdn == idd.fqdn);
    BOOST_CHECK(dom.last_transfer.isnull());
    BOOST_CHECK(dom.registered == idd.creation_time);
    BOOST_CHECK(dom.registrant_handle == idd.registrant.handle);
    BOOST_CHECK(dom.registrar_handle == idd.create_registrar_handle);
    //Jiri: others?
}

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_wrong_handle, test_domain_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(wrong_fqdn);
        BOOST_ERROR("domain handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidLabel& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct wrong_zone_fixture
: test_registrar_fixture, test_registrant_fixture, test_contact_fixture
{
    std::string test_fqdn_bad_zone; //separate so a different fixture

    wrong_zone_fixture()
    : test_registrar_fixture(),
      test_registrant_fixture(),
      test_contact_fixture(),
      test_fqdn_bad_zone("aaa") // !!! hardcoded
    {
        std::vector<std::string> list = impl.get_managed_zone_list();
        BOOST_REQUIRE(list.end() == std::find(list.begin(), list.end(), test_fqdn_bad_zone));
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_wrong_zone, wrong_zone_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(test_fqdn_bad_zone);
        BOOST_ERROR("unreported managed zone");
    }
    catch(const Registry::WhoisImpl::UnmanagedZone& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct many_labels_fixture
: test_registrar_fixture, test_registrant_fixture, test_contact_fixture
{
    std::vector<std::string> domain_list;
    std::string prepare_zone(Fred::OperationContext& ctx, const std::string& zone)
    {
        Fred::Zone::Data zone_data;
        try
        {
            zone_data = Fred::Zone::get_zone(ctx, zone);
        }
        catch(const Fred::Zone::Exception& ex)
        {
            BOOST_ERROR("test zone was not created properly");
        }
        std::ostringstream labeled_zone;
        for(unsigned int i=0; i < 256; ++i) // !!!
        {
            labeled_zone << "1.";
        }
        labeled_zone << zone_data.name;
        return labeled_zone.str();
    }

    many_labels_fixture()
    : test_registrar_fixture(),
      test_registrant_fixture(),
      test_contact_fixture()
    {
        Fred::OperationContext ctx;
        std::vector<std::string> zone_seq = ::Whois::get_managed_zone_list(ctx);
        for(std::vector<std::string>::iterator it = zone_seq.begin(); it != zone_seq.end(); ++it)
        {
            domain_list.push_back(prepare_zone(ctx, *it));
        }
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_many_labels, many_labels_fixture)
{
    for(std::vector<std::string>::iterator it = domain_list.begin(); it != domain_list.end(); ++it)
    {
        try
        {
            Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(*it);
            BOOST_ERROR("permitted label number is wrong");
        }
        catch(const Registry::WhoisImpl::TooManyLabels& ex)
        {
            BOOST_CHECK(true);
            BOOST_MESSAGE(boost::diagnostic_information(ex));
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_no_handle, test_domain_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(no_fqdn);
        BOOST_ERROR("unreported dangling domain");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_invalid_handle, test_domain_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(invalid_fqdn);
        BOOST_ERROR("domain checker rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidLabel& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

struct invalid_unmanaged_fixture
: wrong_zone_fixture
{
    std::string invalid_unmanaged_fqdn;

    invalid_unmanaged_fixture()
    : wrong_zone_fixture()//do not create bad_zone domain for it
    {
        std::ostringstream prefix;
        for(unsigned int i=0; i < 256; ++i)//exceed the size of valid label
        {
            prefix << "1";//invalid part
        }
        prefix << '.' << test_fqdn_bad_zone;//unmanaged part
        invalid_unmanaged_fqdn = prefix.str();
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_invalid_unmanaged, invalid_unmanaged_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(invalid_unmanaged_fqdn);
        BOOST_ERROR("domain must have invalid label and unmanaged zone");
    }
    catch(const Registry::WhoisImpl::InvalidLabel& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
    catch(const Registry::WhoisImpl::UnmanagedZone& ex)
    {
        BOOST_ERROR("domain must check name validity first");
    }
}

struct unmanaged_toomany_fixture
: wrong_zone_fixture
{
    std::string unmanaged_toomany_fqdn;

    unmanaged_toomany_fixture()
    : wrong_zone_fixture()
    {
        std::ostringstream prefix;
        for(unsigned int i=0; i < 20; ++i) // !!!
        {
            prefix << "1.";//toomany part
        }
        prefix << test_fqdn_bad_zone; //unmanaged zone part
        unmanaged_toomany_fqdn = prefix.str();
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_unmanaged_toomany, unmanaged_toomany_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(unmanaged_toomany_fqdn);
        BOOST_ERROR("domain must have unmanaged zone and exceeded number of labels");
    }
    catch(const Registry::WhoisImpl::UnmanagedZone& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
    catch(const Registry::WhoisImpl::TooManyLabels& ex)
    {
        BOOST_ERROR("domain must check managed zone first");
    }
}

struct invalid_toomany_fixture
: whois_impl_instance_fixture
{
    std::vector<std::string> domain_list;

    std::string prepare_zone(Fred::OperationContext& ctx, const std::string& zone)
    {
        Fred::Zone::Data zone_data;
        try
        {
            zone_data = Fred::Zone::get_zone(ctx, zone);
        }
        catch(const Fred::Zone::Exception& ex)
        {
            BOOST_ERROR("test zone was not created properly");
        }
        std::ostringstream labeled_zone, invalid_offset;
        for(unsigned int i=0; i < 256; ++i) // !!!
        {
            labeled_zone << "1.";// invalid + toomany part
        }
        labeled_zone << zone_data.name;
        return labeled_zone.str();
    }

    invalid_toomany_fixture()
    : whois_impl_instance_fixture()
    {
        Fred::OperationContext ctx;
        std::vector<std::string> zone_seq = ::Whois::get_managed_zone_list(ctx);
        domain_list.reserve(zone_seq.size());
        for(std::vector<std::string>::iterator it = zone_seq.begin(); it != zone_seq.end(); ++it)
        {
            domain_list.push_back(prepare_zone(ctx, *it));
        }
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_invalid_toomany, invalid_toomany_fixture)
{
    for(std::vector<std::string>::iterator it = domain_list.begin(); it != domain_list.end(); ++it)
    {
        try
        {
            Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(*it);
            BOOST_ERROR("domain must have invalid handle and exceeded number of labels");
        }
        catch(const Registry::WhoisImpl::InvalidLabel& ex)
        {
            BOOST_CHECK(true);
            BOOST_MESSAGE(boost::diagnostic_information(ex));
        }
        catch(const Registry::WhoisImpl::TooManyLabels& ex)
        {
            BOOST_ERROR("domain must check name validity first");
        }
    }
}

struct invalid_unmanaged_toomany_fixture
: wrong_zone_fixture
{
    std::string invalid_unmanaged_toomany_fqdn;

    invalid_unmanaged_toomany_fixture()
    : wrong_zone_fixture()
    {
        std::ostringstream prefix;
        for(unsigned int i=0; i < 256; ++i)
        {
            prefix << "1."; // invalid + toomany part
        }
        prefix << test_fqdn_bad_zone; //unmanaged zone part
        invalid_unmanaged_toomany_fqdn = prefix.str();
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_invalid_unmanaged_toomany, invalid_unmanaged_toomany_fixture)
{
    try
    {
        Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(invalid_unmanaged_toomany_fqdn);
        BOOST_ERROR("domain must have invalid handle, unmanaged zone and exceeded number of labels");
    }
    catch(const Registry::WhoisImpl::InvalidLabel& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
    catch(const Registry::WhoisImpl::UnmanagedZone& ex)
    {
        BOOST_ERROR("domain must check name validity first");
    }
    catch(const Registry::WhoisImpl::TooManyLabels& ex)
    {
        BOOST_ERROR("domain must check name validity first, then managed zone");
    }
}

struct delete_candidate_fixture //Jiri: check carefully
: test_registrar_fixture, test_registrant_fixture, test_contact_fixture
{
    std::string delete_fqdn;
    std::string delete_status;

    delete_candidate_fixture()
    : test_registrar_fixture(),
      test_registrant_fixture(),
      test_contact_fixture(),
      delete_fqdn(std::string("test-delete") + xmark + ".cz"),
      delete_status("deleteCandidate")
    {
        Fred::OperationContext ctx;
        Fred::CreateDomain(delete_fqdn, test_registrar_handle, test_registrant_handle)
            .set_admin_contacts(Util::vector_of<std::string>(test_admin))
            .exec(ctx);
        ctx.commit_transaction();
        ctx.get_conn().exec_params(
            "UPDATE domain_history "
            "SET exdate = now() - "
                "(SELECT val::int * '1 day'::interval "
                    "FROM enum_parameters"
                    "WHERE name = 'expiration_registration_protection_period')"
            "WHERE id = (SELECT id FROM object_registry WHERE name = $1::text)"

            "UPDATE domain"
            "SET exdate = now() -"
                "(SELECT val::int * '1 day'::interval"
                    "FROM enum_parameters"
                    "WHERE name = 'expiration_registration_protection_period')"
            "WHERE id = (SELECT id FROM object_registry WHERE name = $1::text)",
            Database::query_param_list(delete_fqdn));
        Fred::InfoDomainOutput dom = Fred::InfoDomainByHandle(delete_fqdn).exec(ctx, impl.output_timezone);
        Fred::PerformObjectStateRequest(dom.info_domain_data.id).exec(ctx);
        BOOST_MESSAGE(delete_fqdn);
    }
};

BOOST_FIXTURE_TEST_CASE(get_domain_by_handle_delete_candidate, delete_candidate_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoDomainData idd = Fred::InfoDomainByHandle(delete_fqdn).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone).info_domain_data;
    Registry::WhoisImpl::Domain dom = impl.get_domain_by_handle(delete_fqdn);
    BOOST_CHECK(dom.fqdn == idd.fqdn);
    BOOST_CHECK(dom.changed.isnull());
    BOOST_CHECK(dom.last_transfer.isnull());
    BOOST_CHECK(dom.statuses.end() != std::find(dom.statuses.begin(), dom.statuses.end(), delete_status));
}

BOOST_AUTO_TEST_SUITE_END()//get_domain_by_handle
