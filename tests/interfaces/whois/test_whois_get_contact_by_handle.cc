#include "tests/interfaces/whois/fixture_common.h"
#include "util/random_data_generator.h"
//registrar!
BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_contact_by_handle);

struct test_contact_fixture
: whois_impl_instance_fixture
{
    Fred::OperationContext ctx;
    const Fred::InfoRegistrarData registrar;
    const Fred::InfoContactData contact;

    test_contact_fixture()
    : registrar(
          Test::exec(Fred::CreateRegistrar("REG-FOOBAR"), ctx)
      ),
      contact(
          Test::exec(
                  Fred::CreateContact("CONTACT", registrar.handle) //both!
                      .set_place(Fred::Contact::PlaceAddress(
                                     "Str1",
                                     Optional< std::string >(),//simplier?
                                     Optional< std::string >(),
                                     "Praha",
                                     Optional< std::string >(),
                                     "11150",
                                     "CZ"
                                 )
                    ), ctx)
      )
      //contact.handle(std::string("TEST-CONTACT-HANDLE")+xmark),
    {
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_contact_by_handle, test_contact_fixture)
{
    Registry::WhoisImpl::Contact con = impl.get_contact_by_handle(contact.handle);

    BOOST_CHECK(con.address.city  == contact.place.get_value_or_default().city);
    BOOST_CHECK(con.address.country_code == contact.place.get_value_or_default().country);
    BOOST_CHECK(con.address.postal_code == contact.place.get_value_or_default().postalcode);
    BOOST_CHECK(con.address.street1 == contact.place.get_value_or_default().street1);
}

BOOST_FIXTURE_TEST_CASE(get_contact_by_handle_no_contact, test_contact_fixture)
{
    try
    {
        Registry::WhoisImpl::Contact con = impl.get_contact_by_handle("fine-handle");
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
        Registry::WhoisImpl::Contact con = impl.get_contact_by_handle("");//[a-zA-Z0-9_:.-]{1,63}
        BOOST_ERROR("contact handle rule is wrong");
    }
    catch(const Registry::WhoisImpl::InvalidHandle& ex)
    {
        BOOST_CHECK(true);
        BOOST_MESSAGE(boost::diagnostic_information(ex));
    }
}

BOOST_AUTO_TEST_SUITE_END();//get_contact_by_handle
BOOST_AUTO_TEST_SUITE_END();//TestWhois
