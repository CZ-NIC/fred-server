//registrar!
BOOST_AUTO_TEST_SUITE(get_contact_by_handle);

struct test_contact_fixture
: test_registrar_fixture
{
    std::string test_contact_handle;
    std::string no_contact_handle;
    std::string wrong_contact_handle;
    Fred::Contact::PlaceAddress contact_place;

    test_contact_fixture()
    : test_registrar_fixture(),
      test_contact_handle(std::string("TEST-CONTACT-HANDLE")+xmark),
      no_contact_handle("fine-handle"),
      wrong_contact_handle("")
    {
        Fred::OperationContext ctx;
        contact_place.city = "Praha";
        contact_place.country = "CZ";
        contact_place.postalcode = "11150";
        contact_place.street1 = "STR1";
        Fred::CreateContact(test_contact_handle, test_registrar_handle)
            .set_place(contact_place).exec(ctx);
        ctx.commit_transaction();
        BOOST_MESSAGE(test_contact_handle);
    }
};


BOOST_FIXTURE_TEST_CASE(get_contact_by_handle, test_contact_fixture)
{
    Fred::OperationContext ctx;
    Fred::InfoContactData icd = Fred::InfoContactByHandle(test_contact_handle).exec(ctx, Registry::WhoisImpl::Server_impl::output_timezone).info_contact_data;
    Registry::WhoisImpl::Contact con = impl.get_contact_by_handle(test_contact_handle);

    BOOST_CHECK(con.address.city  == icd.place.get_value_or_default().city);
    BOOST_CHECK(con.address.country_code == icd.place.get_value_or_default().country);
    BOOST_CHECK(con.address.postal_code == icd.place.get_value_or_default().postalcode);
    BOOST_CHECK(con.address.street1 == icd.place.get_value_or_default().street1);
}

BOOST_FIXTURE_TEST_CASE(get_contact_by_handle_no_contact, test_contact_fixture)
{
    try
    {
        Registry::WhoisImpl::Contact con = impl.get_contact_by_handle(no_contact_handle);
        BOOST_ERROR("unreported dangling contact");
    }
    catch(const Registry::WhoisImpl::ObjectNotExists& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_FIXTURE_TEST_CASE(get_contact_by_handle_wrong_contact, test_contact_fixture)
{
    try
    {
        Registry::WhoisImpl::Contact con = impl.get_contact_by_handle(wrong_contact_handle);//[a-zA-Z0-9_:.-]{1,63}
        BOOST_ERROR("contact handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//get_contact_by_handle
