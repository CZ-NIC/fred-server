#include "tests/interfaces/whois/fixture_common.h"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_contact_by_handle);

struct test_contact_fixture
: whois_impl_instance_fixture
{
    boost::posix_time::ptime now_utc;
    Fred::InfoContactData contact;

    test_contact_fixture()
    {
        Fred::OperationContextCreator ctx;
        const Fred::InfoRegistrarData registrar = Test::registrar::make(ctx);
        contact = Test::exec(
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

    BOOST_CHECK(con.fax                                  == contact.fax.get_value_or_default());
    BOOST_CHECK(con.name                                 == contact.name.get_value_or_default());
    BOOST_CHECK(con.phone                                == contact.telephone.get_value_or_default());
    BOOST_CHECK(con.email                                == contact.email.get_value_or_default());
    BOOST_CHECK(con.handle                               == contact.handle);
    BOOST_CHECK(con.created                              == now_utc);
    BOOST_CHECK(con.vat_number                           == contact.vat.get_value_or_default());
    BOOST_CHECK(con.notify_email                         == contact.notifyemail.get_value_or_default());
    BOOST_CHECK(con.organization                         == contact.organization.get_value_or_default());
    BOOST_CHECK(con.creating_registrar                   == contact.create_registrar_handle);
    BOOST_CHECK(con.sponsoring_registrar                 == contact.sponsoring_registrar_handle);
    BOOST_CHECK(con.changed.get_value_or_default()       == contact.update_time.get_value_or_default());
    BOOST_CHECK(con.last_transfer.get_value_or_default() == contact.transfer_time.get_value_or_default());
    BOOST_CHECK(con.address.city                         == contact.place.get_value_or_default().city);
    BOOST_CHECK(con.address.street1                      == contact.place.get_value_or_default().street1);
    BOOST_CHECK(con.address.postal_code                  == contact.place.get_value_or_default().postalcode);
    BOOST_CHECK(con.address.country_code                 == contact.place.get_value_or_default().country);
    BOOST_CHECK(con.disclose_organization                == contact.discloseorganization);
    BOOST_CHECK(con.disclose_name                        == contact.disclosename);
    BOOST_CHECK(con.disclose_address                     == contact.discloseaddress);
    BOOST_CHECK(con.disclose_phone                       == contact.disclosetelephone);
    BOOST_CHECK(con.disclose_fax                         == contact.disclosefax);
    BOOST_CHECK(con.disclose_email                       == contact.discloseemail);
    BOOST_CHECK(con.disclose_notify_email                == contact.disclosenotifyemail);
    BOOST_CHECK(con.disclose_identification              == contact.discloseident);
    BOOST_CHECK(con.disclose_vat_number                  == contact.disclosevat);
}


struct test_contact_discloseflags_fixture
: whois_impl_instance_fixture
{
    boost::posix_time::ptime now_utc;
    Fred::InfoContactData contact_show;
    Fred::InfoContactData contact_hide;

    test_contact_discloseflags_fixture()
    {
        Fred::OperationContextCreator ctx;
        const Fred::InfoRegistrarData registrar = Test::registrar::make(ctx);
        contact_show = Test::exec(
                    Test::generate_test_data(
                        Test::CreateX_factory<Fred::CreateContact>().make(registrar.handle))
                                .set_disclosename(true)
                                .set_discloseorganization(true)
                                .set_discloseaddress(true)
                                .set_disclosetelephone(true)
                                .set_disclosefax(true)
                                .set_discloseemail(true)
                                .set_disclosevat(true)
                                .set_discloseident(true)
                                .set_disclosenotifyemail(true),
                        ctx
                   );
        contact_hide = Test::exec(
                    Test::generate_test_data(
                        Test::CreateX_factory<Fred::CreateContact>().make(registrar.handle))
                                .set_disclosename(false)
                                .set_discloseorganization(false)
                                .set_discloseaddress(false)
                                .set_disclosetelephone(false)
                                .set_disclosefax(false)
                                .set_discloseemail(false)
                                .set_disclosevat(false)
                                .set_discloseident(false)
                                .set_disclosenotifyemail(false),
                        ctx
                   );

        now_utc = boost::posix_time::time_from_string(
                static_cast<std::string>(ctx.get_conn().exec("SELECT now()::timestamp")[0][0])
        );
        ctx.commit_transaction();
    }
};


BOOST_FIXTURE_TEST_CASE(get_contact_by_handle_discloseflags, test_contact_discloseflags_fixture)
{
    Registry::WhoisImpl::Contact c1 = impl.get_contact_by_handle(contact_show.handle);

    BOOST_CHECK(c1.disclose_organization                == contact_show.discloseorganization);
    BOOST_CHECK(c1.disclose_name                        == contact_show.disclosename);
    BOOST_CHECK(c1.disclose_address                     == contact_show.discloseaddress);
    BOOST_CHECK(c1.disclose_phone                       == contact_show.disclosetelephone);
    BOOST_CHECK(c1.disclose_fax                         == contact_show.disclosefax);
    BOOST_CHECK(c1.disclose_email                       == contact_show.discloseemail);
    BOOST_CHECK(c1.disclose_notify_email                == contact_show.disclosenotifyemail);
    BOOST_CHECK(c1.disclose_identification              == contact_show.discloseident);
    BOOST_CHECK(c1.disclose_vat_number                  == contact_show.disclosevat);

    Registry::WhoisImpl::Contact c2 = impl.get_contact_by_handle(contact_hide.handle);

    BOOST_CHECK(c2.disclose_organization                == contact_hide.discloseorganization);
    BOOST_CHECK(c2.disclose_name                        == contact_hide.disclosename);
    BOOST_CHECK(c2.disclose_address                     == contact_hide.discloseaddress);
    BOOST_CHECK(c2.disclose_phone                       == contact_hide.disclosetelephone);
    BOOST_CHECK(c2.disclose_fax                         == contact_hide.disclosefax);
    BOOST_CHECK(c2.disclose_email                       == contact_hide.discloseemail);
    BOOST_CHECK(c2.disclose_notify_email                == contact_hide.disclosenotifyemail);
    BOOST_CHECK(c2.disclose_identification              == contact_hide.discloseident);
    BOOST_CHECK(c2.disclose_vat_number                  == contact_hide.disclosevat);
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
