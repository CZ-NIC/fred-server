//registrar!
//contact could be empty
BOOST_AUTO_TEST_SUITE(get_keyset_by_handle)

struct get_keyset_by_handle_fixture
: test_registrar_fixture, test_contact_fixture
{
    std::string test_keyset_handle;
    std::string no_keyset_handle;
    std::string wrong_keyset_handle;

    get_keyset_by_handle_fixture()
    : test_registrar_fixture(),
      test_contact_fixture(),
      test_keyset_handle(std::string("TEST-KEYSET-HANDLE")+xmark),
      no_keyset_handle("fine-keyset-handle"),
      wrong_keyset_handle("")
    {
        Fred::OperationContext ctx;
        Fred::CreateKeyset(test_keyset_handle, test_registrar_handle)
            .set_dns_keys(Util::vector_of<Fred::DnsKey>(Fred::DnsKey(42, 777, 13, "any-key")))//what key has to be here?
            .set_tech_contacts(Util::vector_of<std::string>(test_admin)(test_contact))
            .exec(ctx);
        ctx.commit_transaction();
        BOOST_MESSAGE(test_keyset_handle);
    }
};

BOOST_FIXTURE_TEST_CASE(get_keyset_by_handle, get_keyset_by_handle_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoKeysetData ikd = Fred::InfoKeysetByHandle(test_keyset_handle).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone).info_keyset_data;
    Registry::WhoisImpl::KeySet ks = impl.get_keyset_by_handle(test_keyset_handle);

    BOOST_CHECK(ks.changed.isnull());//new has to be unchanged
    BOOST_CHECK(ks.created == ikd.creation_time);
    BOOST_CHECK(ks.dns_keys.at(0).alg == ikd.dns_keys.at(0).get_alg());
    BOOST_CHECK(ks.dns_keys.at(0).flags == ikd.dns_keys.at(0).get_flags());
    BOOST_CHECK(ks.dns_keys.at(0).protocol == ikd.dns_keys.at(0).get_protocol());
    BOOST_CHECK(ks.dns_keys.at(0).public_key == ikd.dns_keys.at(0).get_key());
    BOOST_CHECK(ks.handle == ikd.handle);
    BOOST_CHECK(ks.last_transfer.isnull());//new has to have no transfer
    BOOST_CHECK(ks.registrar_handle == ikd.create_registrar_handle);
    BOOST_CHECK(ks.tech_contact_handles.at(0) == ikd.tech_contacts.at(0).handle);
}

BOOST_FIXTURE_TEST_CASE(get_keyset_by_handle_no_keyset, get_keyset_by_handle_fixture)
{
    try
    {
        Registry::WhoisImpl::KeySet ks = impl.get_keyset_by_handle(no_keyset_handle);
        BOOST_ERROR("unreported dangling keyset");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_keyset_by_handle_wrong_nsset, get_keyset_by_handle_fixture)
{
    try
    {
        Registry::WhoisImpl::KeySet ks = impl.get_keyset_by_handle(wrong_keyset_handle);
        BOOST_ERROR("keyset handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_keyset_by_handle
