#include "tests/interfaces/whois/fixture_common.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_contact_by_handle);

struct test_contact_fixture
: whois_impl_instance_fixture
{
    boost::posix_time::ptime now_utc;

    test_contact_fixture()
    {
        Fred::OperationContextCreator ctx;
        const Fred::InfoRegistrarData registrar = Test::registrar::make(ctx);
        const Fred::InfoContactData contact = Test::exec(
                    Test::generate_test_data(
                        Test::CreateX_factory<Fred::CreateContact>().make(registrar.handle)),
                    ctx);
        now_utc = boost::posix_time::time_from_string(
                      static_cast<std::string>(
                          ctx.get_conn().exec("SELECT now()::timestamp")[0][0]));
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
    BOOST_CHECK(con.created                     == now_utc);
    BOOST_CHECK(con.vat_number                  == contact.vat.get_value_or_default());
    BOOST_CHECK(con.notify_email                == contact.notifyemail.get_value_or_default());
    BOOST_CHECK(con.organization                == contact.organization.get_value_or_default());
    BOOST_CHECK(con.creating_registrar          == contact.create_registrar_handle);
    BOOST_CHECK(con.sponsoring_registrar        == contact.sponsoring_registrar_handle);
    BOOST_CHECK(con.changed.get_value_or_default()       == contact.update_time.get_value_or_default());
    BOOST_CHECK(con.last_transfer.get_value_or_default() == contact.transfer_time.get_value_or_default());
    BOOST_CHECK(con.address.city                == contact.place.get_value_or_default().city);
    BOOST_CHECK(con.address.street1             == contact.place.get_value_or_default().street1);
    BOOST_CHECK(con.address.postal_code         == contact.place.get_value_or_default().postalcode);
    BOOST_CHECK(con.address.country_code        == contact.place.get_value_or_default().country);
}

BOOST_FIXTURE_TEST_CASE(get_contact_by_handle_no_contact, test_contact_fixture)
{
    BOOST_CHECK_THROW(impl.get_contact_by_handle("fine-handle"), Registry::WhoisImpl::ObjectNotExists);
}

BOOST_FIXTURE_TEST_CASE(get_contact_by_handle_wrong_contact, test_contact_fixture)
{
    BOOST_CHECK_THROW(impl.get_contact_by_handle(""), Registry::WhoisImpl::InvalidHandle);
}

BOOST_AUTO_TEST_SUITE_END();//get_contact_by_handle
BOOST_AUTO_TEST_SUITE_END();//TestWhois
