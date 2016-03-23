//registrar!
//contact!
BOOST_AUTO_TEST_SUITE(get_keysets_by_tech_c)

struct get_keysets_by_tech_c_fixture
: test_registrar_fixture, test_contact_fixture
{
    std::string test_keyset_handle;
    std::string test_no_handle;
    std::string test_wrong_handle;
    unsigned long test_limit;

    get_keysets_by_tech_c_fixture()
    : test_registrar_fixture(),
      test_contact_fixture(),
      test_keyset_handle(std::string("TEST_KEYSET_HANDLE") + xmark),
      test_no_handle("fine-tech-c-handle"),
      test_wrong_handle(""),
      test_limit(10)
    {
        Fred::OperationContext ctx;
        for(unsigned long i = 0; i < test_limit; ++i)
        {
            std::ostringstream test_handles;
            test_handles << test_keyset_handle << i;

            Fred::CreateKeyset(test_handles.str(), test_registrar_handle + boost::lexical_cast<std::string>(i))
                .set_dns_keys(Util::vector_of<Fred::DnsKey>(Fred::DnsKey(42, 777, 13, "any-key")))
                .set_tech_contacts(Util::vector_of<std::string>(test_admin)(test_contact))
                .exec(ctx);
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c, get_keysets_by_tech_c_fixture)
{
    Fred::OperationContext ctx;
    std::vector<Fred::InfoKeysetOutput> v_iko = Fred::InfoKeysetByTechContactHandle(test_admin).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone);
    Registry::WhoisImpl::KeySetSeq ks_s = impl.get_keysets_by_tech_c(test_admin, test_limit);
    for(unsigned long i = 0; i < test_limit; ++i)
    {
        std::vector<Fred::InfoKeysetOutput>::iterator it = v_iko.begin(), end = v_iko.end();
        while(it != end)
        {
            if(it->info_keyset_data.handle == ks_s.content.at(0).handle) break;
            ++it;
        }
        BOOST_REQUIRE(ks_s.content.at(0).handle == it->info_keyset_data.handle);
        Fred::InfoKeysetData found = it->info_keyset_data;

        BOOST_CHECK(ks_s.content.at(0).changed.isnull());
        BOOST_CHECK(ks_s.content.at(0).created == found.creation_time);
        BOOST_CHECK(ks_s.content.at(0).dns_keys.at(0).alg == found.dns_keys.at(0).get_alg());
        BOOST_CHECK(ks_s.content.at(0).dns_keys.at(0).flags == found.dns_keys.at(0).get_flags());
        BOOST_CHECK(ks_s.content.at(0).dns_keys.at(0).protocol == found.dns_keys.at(0).get_protocol());
        BOOST_CHECK(ks_s.content.at(0).dns_keys.at(0).public_key == found.dns_keys.at(0).get_key());
        BOOST_CHECK(ks_s.content.at(0).last_transfer.isnull());
        BOOST_CHECK(ks_s.content.at(0).registrar_handle == found.create_registrar_handle);
        BOOST_CHECK(ks_s.content.at(0).tech_contact_handles.at(0) == found.tech_contacts.at(0).handle);
    }
}

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c_no_contact, get_keysets_by_tech_c_fixture)
{
    try
    {
        Registry::WhoisImpl::KeySetSeq ks_s = impl.get_keysets_by_tech_c(test_no_handle, test_limit);
        BOOST_ERROR("unreported dangling KeySets");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_keysets_by_tech_c_wrong_contact, get_keysets_by_tech_c_fixture)
{
    try
    {
        Registry::WhoisImpl::KeySetSeq ks_s = impl.get_keysets_by_tech_c(test_wrong_handle, test_limit);
        BOOST_ERROR("tech contact handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_keysets_by_tech_c
