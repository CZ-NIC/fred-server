/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

/**
 *  @file
 */

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>

#include "tests/interfaces/epp/util.h"
#include "tests/interfaces/epp/contact/fixture.h"

#include "src/epp/contact/contact_create_impl.h"

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(ContactCreateImpl)

BOOST_FIXTURE_TEST_CASE(create_invalid_registrar_id, has_registrar)
{
    const Epp::ContactCreateInputData contact_data(
        "contacthandle",
        "Jan Novak Jr.",
        "",
        "ulice 1",
        "ulice 2",
        "ulice 3",
        "mesto",
        "hejtmanstvi",
        "12345",
        "CZ",
        "+420 123 456 789",
        "+420 987 654 321",
        "jan@novak.novak",
        "jan.notify@novak.novak",
        "MyVATstring",
        "",
        Nullable<Epp::IdentType::Enum>(),
        "authInfo123",
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true
    );

    BOOST_CHECK_THROW(
        Epp::contact_create_impl(
            ctx,
            contact_data,
            0 /* <== !!! */,
            42
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(create_fail_handle_format, has_registrar)
{
    const Epp::ContactCreateInputData contact_data(
        "contacthandle1?",
        "Jan Novak Jr.",
        "",
        "ulice 1",
        "ulice 2",
        "ulice 3",
        "mesto",
        "hejtmanstvi",
        "12345",
        "CZ",
        "+420 123 456 789",
        "+420 987 654 321",
        "jan@novak.novak",
        "jan.notify@novak.novak",
        "MyVATstring",
        "",
        Nullable<Epp::IdentType::Enum>(),
        "authInfo123",
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true
    );

    try {
        Epp::contact_create_impl(
            ctx,
            contact_data,
            registrar.id,
            42 /* TODO */
        );
    } catch(...) {
        Test::check_correct_aggregated_exception_was_thrown(Epp::Error::of_scalar_parameter(Epp::Param::contact_handle, Epp::Reason::bad_format_contact_handle));
    }
}

BOOST_FIXTURE_TEST_CASE(create_fail_already_existing, has_contact)
{
    const Epp::ContactCreateInputData contact_data(
        contact.handle,
        "Jan Novak Jr.",
        "",
        "ulice 1",
        "ulice 2",
        "ulice 3",
        "mesto",
        "hejtmanstvi",
        "12345",
        "CZ",
        "+420 123 456 789",
        "+420 987 654 321",
        "jan@novak.novak",
        "jan.notify@novak.novak",
        "MyVATstring",
        "",
        Nullable<Epp::IdentType::Enum>(),
        "authInfo123",
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true
    );

    BOOST_CHECK_THROW(
        Epp::contact_create_impl(
            ctx,
            contact_data,
            registrar.id,
            42 /* TODO */
        ),
        Epp::ObjectExists
    );
}

BOOST_FIXTURE_TEST_CASE(create_fail_protected_handle, has_contact)
{
    {   /* fixture */
        Fred::DeleteContactByHandle(contact.handle).exec(ctx);
    }

    const Epp::ContactCreateInputData contact_data(
        contact.handle,
        "Jan Novak Jr.",
        "",
        "ulice 1",
        "ulice 2",
        "ulice 3",
        "mesto",
        "hejtmanstvi",
        "12345",
        "CZ",
        "+420 123 456 789",
        "+420 987 654 321",
        "jan@novak.novak",
        "jan.notify@novak.novak",
        "MyVATstring",
        "",
        Nullable<Epp::IdentType::Enum>(),
        "authInfo123",
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true
    );

    try {
        Epp::contact_create_impl(
            ctx,
            contact_data,
            registrar.id,
            42 /* TODO */
        );
    } catch (...) {
        Test::check_correct_aggregated_exception_was_thrown(Epp::Error::of_scalar_parameter(Epp::Param::contact_handle, Epp::Reason::protected_period));
    }
}

BOOST_FIXTURE_TEST_CASE(create_fail_nonexistent_countrycode, has_registrar)
{
    const Epp::ContactCreateInputData contact_data(
        "contacthandle1",
        "Jan Novak Jr.",
        "",
        "ulice 1",
        "ulice 2",
        "ulice 3",
        "mesto",
        "hejtmanstvi",
        "12345",
        "1Z9", /* <- !!! */
        "+420 123 456 789",
        "+420 987 654 321",
        "jan@novak.novak",
        "jan.notify@novak.novak",
        "MyVATstring",
        "",
        Nullable<Epp::IdentType::Enum>(),
        "authInfo123",
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true
    );

    try{
        Epp::contact_create_impl(
            ctx,
            contact_data,
            registrar.id,
            42 /* TODO */
        );
    } catch (...) {
        Test::check_correct_aggregated_exception_was_thrown(Epp::Error::of_scalar_parameter(Epp::Param::contact_cc, Epp::Reason::country_notexist));
    }
}

void check_equal(const Epp::ContactCreateInputData& create_data, const Fred::InfoContactData& info_data) {
    BOOST_CHECK_EQUAL( boost::to_upper_copy( create_data.handle ), info_data.handle );
    BOOST_CHECK_EQUAL( create_data.name,                info_data.name.get_value_or_default() );
    BOOST_CHECK_EQUAL( create_data.organization,        info_data.organization.get_value_or_default() );

    BOOST_CHECK_EQUAL( create_data.street1,             info_data.place.get_value_or_default().street1 );
    BOOST_CHECK_EQUAL( create_data.street2,             info_data.place.get_value_or_default().street2.get_value_or_default() );
    BOOST_CHECK_EQUAL( create_data.street3,             info_data.place.get_value_or_default().street3.get_value_or_default() );
    BOOST_CHECK_EQUAL( create_data.city,                info_data.place.get_value_or_default().city );
    BOOST_CHECK_EQUAL( create_data.postal_code,         info_data.place.get_value_or_default().postalcode );
    BOOST_CHECK_EQUAL( create_data.state_or_province,   info_data.place.get_value_or_default().stateorprovince.get_value_or_default() );
    BOOST_CHECK_EQUAL( create_data.country_code,        info_data.place.get_value_or_default().country );
    BOOST_CHECK_EQUAL( create_data.telephone,           info_data.telephone.get_value_or_default() );
    BOOST_CHECK_EQUAL( create_data.fax,                 info_data.fax.get_value_or_default() );
    BOOST_CHECK_EQUAL( create_data.email,               info_data.email.get_value_or_default() );
    BOOST_CHECK_EQUAL( create_data.notify_email,        info_data.notifyemail.get_value_or_default() );
    BOOST_CHECK_EQUAL( create_data.VAT,                 info_data.vat.get_value_or_default() );
    BOOST_CHECK_EQUAL( create_data.ident,               info_data.ssn.get_value_or_default() );

    BOOST_CHECK_EQUAL(
        create_data.identtype.isnull()
            ?   ""
            :   Epp::to_db_handle( create_data.identtype.get_value() ),
        info_data.ssntype.get_value_or_default()
    );

    BOOST_CHECK_EQUAL( create_data.authinfo,                info_data.authinfopw );
    BOOST_CHECK_EQUAL( create_data.disclose_name,           info_data.disclosename );
    BOOST_CHECK_EQUAL( create_data.disclose_organization,   info_data.discloseorganization );
    BOOST_CHECK_EQUAL( create_data.disclose_address,        info_data.discloseaddress );
    BOOST_CHECK_EQUAL( create_data.disclose_telephone,      info_data.disclosetelephone );
    BOOST_CHECK_EQUAL( create_data.disclose_fax,            info_data.disclosefax );
    BOOST_CHECK_EQUAL( create_data.disclose_email,          info_data.discloseemail );
    BOOST_CHECK_EQUAL( create_data.disclose_VAT,            info_data.disclosevat );
    BOOST_CHECK_EQUAL( create_data.disclose_ident,          info_data.discloseident );
    BOOST_CHECK_EQUAL( create_data.disclose_notify_email,   info_data.disclosenotifyemail );
}

BOOST_FIXTURE_TEST_CASE(create_ok_all_data, has_registrar)
{
    const Epp::ContactCreateInputData contact_data(
        "contacthandle1",
        "Jan Novak Jr.",
        "",
        "ulice 1",
        "ulice 2",
        "ulice 3",
        "mesto",
        "hejtmanstvi",
        "12345",
        "CZ",
        "+420 123 456 789",
        "+420 987 654 321",
        "jan@novak.novak",
        "jan.notify@novak.novak",
        "MyVATstring",
        "",
        Nullable<Epp::IdentType::Enum>(),
        "authInfo123",
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true
    );

    const Epp::ContactCreateResult result = Epp::contact_create_impl(
        ctx,
        contact_data,
        registrar.id,
        42 /* TODO */
    );

    /* check returned data and db changes */
    {
        const Fred::InfoContactData check_sample = Fred::InfoContactByHandle(contact_data.handle).exec(ctx).info_contact_data;
        BOOST_CHECK_EQUAL( check_sample.id, result.id );
        BOOST_CHECK_EQUAL(
            boost::posix_time::time_from_string(
                static_cast<std::string>(
                    ctx.get_conn().exec("SELECT now() AT TIME ZONE 'utc' AS now_")[0]["now_"]
                )
            ),
            result.crdate
        );
        check_equal(
            contact_data,
            check_sample
        );
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
