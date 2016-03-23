#include "tests/interfaces/whois/fixture_common.h"
#include "util/random_data_generator.h"
//registrar!
BOOST_AUTO_TEST_SUITE(get_contact_by_handle);

struct test_contact_fixture
: whois_impl_instance_fixture
{
    Fred::OperationContext ctx;
    std::string no_contact_handle;
    std::string wrong_contact_handle;
    const Fred::InfoRegistrarData registrar;

    test_contact_fixture()
    : registrar(
          Test::exec(Fred::CreateRegistrar("REG-FOOBAR"), ctx)
      ),
      contact(
          Test::exec(
                  Fred::CreateContact("CONTACT", registrar.handle)
                      .set_place(Fred::Contact::PlaceAddres(
                                     "Str1",
                                     Optional< std::string >(),//simplier?
                                     Optional< std::string >(),
                                     "Praha",
                                     Optional< std::string >(),
                                     "11150",
                                     "CZ"
                                 )
                       )
                      .
                  , ctx)
          )
      )
      test_contact_handle(std::string("TEST-CONTACT-HANDLE")+xmark),
      no_contact_handle("fine-handle"),
      wrong_contact_handle("")
    {
       Fred::CreateContact(test_contact_handle, test_registrar_handle)
        ctx.commit_transaction();
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
