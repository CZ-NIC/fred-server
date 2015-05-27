/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/test/unit_test.hpp>
#include <string>

#include "src/fredlib/contact/checkers.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"

const std::string server_name = "test-contact-checkers";

struct test_contact_checkers_fixture : public Test::Fixture::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string test_contact_handle;
    Fred::InfoContactData contact;

    test_contact_checkers_fixture()
    :   xmark(RandomDataGenerator().xnumstring(6)),
        test_contact_handle(std::string("TEST-CONTACT-HANDLE") + xmark)
    {
        Fred::OperationContext ctx;
        registrar_handle = static_cast< std::string >(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        Fred::Contact::PlaceAddress place;
        place.street1    = "STR1" + xmark;
        place.city       = "Praha";
        place.postalcode = "11150";
        place.country    = "CZ";
        Fred::CreateContact(test_contact_handle, registrar_handle)
            .set_name("TEST-CONTACT NAME" + xmark)
            .set_place(place)
            .set_email("contact" + xmark + "@sezdan.cz")
            .set_telephone("+420." + xmark + "321")
            .exec(ctx);

        contact = Fred::InfoContactByHandle(test_contact_handle).exec(ctx).info_contact_data;
        ctx.commit_transaction();//commit fixture
    }
    ~test_contact_checkers_fixture()
    {}
    typedef boost::mpl::list< Fred::check_contact_name,
                              Fred::check_contact_mailing_address,
                              Fred::check_contact_email_presence,
                              Fred::check_contact_email_validity,
                              Fred::check_contact_phone_presence,
                              Fred::check_contact_phone_validity,
                              Fred::check_contact_fax_validity,
                              Fred::MojeID::check_contact_username,
                              Fred::MojeID::check_contact_birthday_validity > list_of_checks_contact;
    typedef boost::mpl::list< Fred::check_contact_email_availability,
                              Fred::check_contact_phone_availability > list_of_checks_contact_ctx;
    typedef Fred::Check< boost::mpl::list< list_of_checks_contact,
                                           list_of_checks_contact_ctx > > SumCheck;
    typedef Fred::Check< boost::mpl::list< list_of_checks_contact,
                                           list_of_checks_contact_ctx >,
                         Fred::check_wrapper_break_on_first_error > SumCheckWithException;
};


BOOST_FIXTURE_TEST_SUITE(TestContactCheckers, test_contact_checkers_fixture)

/**
 * test call SumCheck
*/
BOOST_AUTO_TEST_CASE(check_all_without_exceptions)
{
    try {
        Fred::OperationContext ctx;
        const SumCheck result(Fred::make_args(contact), Fred::make_args(contact, ctx));
        BOOST_CHECK(result.success());
        BOOST_CHECK(result.Fred::check_contact_name::success());
        BOOST_CHECK(result.Fred::check_contact_mailing_address::success());
        BOOST_CHECK(result.Fred::check_contact_email_presence::success());
        BOOST_CHECK(result.Fred::check_contact_email_validity::success());
        BOOST_CHECK(result.Fred::check_contact_phone_presence::success());
        BOOST_CHECK(result.Fred::check_contact_phone_validity::success());
        BOOST_CHECK(result.Fred::check_contact_fax_validity::success());
        BOOST_CHECK(result.Fred::MojeID::check_contact_username::success());
        BOOST_CHECK(result.Fred::MojeID::check_contact_birthday_validity::success());
        BOOST_CHECK(result.Fred::check_contact_email_availability::success());
        BOOST_CHECK(result.Fred::check_contact_phone_availability::success());
    }
    catch (const std::exception &e) {
        BOOST_FAIL("unexpected exception: " << e.what());
    }
}

/**
 * test call SumCheckWithException
*/
BOOST_AUTO_TEST_CASE(check_all_with_exceptions)
{
    try {
        Fred::OperationContext ctx;
        const SumCheckWithException result(Fred::make_args(contact), Fred::make_args(contact, ctx));
        BOOST_CHECK(result.success());
    }
    catch (const Fred::check_contact_name&) {
        BOOST_FAIL("Fred::check_contact_name failure");
    }
    catch (const Fred::check_contact_mailing_address&) {
        BOOST_FAIL("Fred::check_contact_mailing_address failure");
    }
    catch (const Fred::check_contact_email_presence&) {
        BOOST_FAIL("Fred::check_contact_email_presence failure");
    }
    catch (const Fred::check_contact_email_validity&) {
        BOOST_FAIL("Fred::check_contact_email_validity failure: " + contact.email.get_value_or_default());
    }
    catch (const Fred::check_contact_phone_presence&) {
        BOOST_FAIL("Fred::check_contact_phone_presence failure");
    }
    catch (const Fred::check_contact_phone_validity&) {
        BOOST_FAIL("Fred::check_contact_phone_validity failure");
    }
    catch (const Fred::check_contact_fax_validity&) {
        BOOST_FAIL("Fred::check_contact_fax_validity failure");
    }
    catch (const Fred::MojeID::check_contact_username&) {
        BOOST_FAIL("Fred::MojeID::check_contact_username failure");
    }
    catch (const Fred::MojeID::check_contact_birthday_validity&) {
        BOOST_FAIL("Fred::MojeID::check_contact_birthday_validity");
    }
    catch (const Fred::check_contact_email_availability&) {
        BOOST_FAIL("Fred::check_contact_email_availability failure");
    }
    catch (const Fred::check_contact_phone_availability&) {
        BOOST_FAIL("Fred::check_contact_phone_availability failure");
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestContactCheckers
