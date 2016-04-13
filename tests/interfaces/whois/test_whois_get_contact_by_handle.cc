#include "tests/interfaces/whois/fixture_common.h"
#include "tests/setup/fixtures_utils.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_contact_by_handle);

struct test_contact_fixture
: whois_impl_instance_fixture
{
    Fred::OperationContext ctx;
    const Fred::InfoRegistrarData registrar;
    const Fred::InfoContactData contact;
    const boost::posix_time::ptime now_utc;
    const boost::posix_time::ptime now_prague;

    test_contact_fixture()
    : registrar(Test::registrar::make(ctx)),
      contact(
          Test::exec(
              Test::CreateX_factory<Fred::CreateContact>().make(registrar.handle)
                  .set_place(Fred::Contact::PlaceAddress(
                                "Str1",
                                 Optional<std::string>(),//simplier?
                                 Optional<std::string>(),
                                 "Praha",
                                 Optional<std::string>(),
                                 "11150",
                                 "CZ")),
              ctx)),
      now_utc(boost::posix_time::time_from_string(
                  static_cast<std::string>(ctx.get_conn()
                  .exec("SELECT now()::timestamp")[0][0]))),
      now_prague(boost::posix_time::time_from_string(
                  static_cast<std::string>(ctx.get_conn()
                  .exec("SELECT now() AT TIME ZONE 'Europe/Prague'")[0][0])))
    {
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_contact_by_handle, test_contact_fixture)
{
    Registry::WhoisImpl::Contact con = impl.get_contact_by_handle(contact.handle);

    BOOST_CHECK(con.fax                         == contact.fax.get_value_or_default());
    BOOST_CHECK(con.name                        == contact.name.get_value_or_default());
    BOOST_CHECK(con.phone                       == contact.telephone.get_value_or_default());
    BOOST_CHECK(con.email                       == contact.email.get_value_or_default());
    BOOST_CHECK(con.handle                      == contact.handle);
    BOOST_CHECK(con.created                     == contact.creation_time);
    BOOST_CHECK(con.changed                     == contact.update_time.get_value_or_default());
    BOOST_CHECK(con.vat_number                  == contact.vat.get_value_or_default());
    BOOST_CHECK(con.notify_email                == contact.notifyemail.get_value_or_default());
    BOOST_CHECK(con.organization                == contact.organization.get_value_or_default());
    BOOST_CHECK(con.last_transfer               == contact.transfer_time.get_value_or_default());
    BOOST_CHECK(con.creating_registrar_handle   == contact.create_registrar_handle);
    BOOST_CHECK(con.sponsoring_registrar_handle == contact.sponsoring_registrar_handle);
    BOOST_CHECK(con.address.city                == contact.place.get_value_or_default().city);
    BOOST_CHECK(con.address.street1             == contact.place.get_value_or_default().street1);
    BOOST_CHECK(con.address.postal_code         == contact.place.get_value_or_default().postalcode);
    BOOST_CHECK(con.address.country_code        == contact.place.get_value_or_default().country);
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
