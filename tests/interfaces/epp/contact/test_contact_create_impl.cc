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

#include "tests/interfaces/epp/util.h"
#include "tests/interfaces/epp/contact/fixture.h"

#include "src/epp/disclose_policy.h"
#include "src/epp/contact/contact_create_impl.h"

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>

namespace {

void set_all_items(std::set< Epp::ContactDisclose::Enum > &items)
{
    items.insert(Epp::ContactDisclose::name);
    items.insert(Epp::ContactDisclose::organization);
    items.insert(Epp::ContactDisclose::address);
    items.insert(Epp::ContactDisclose::telephone);
    items.insert(Epp::ContactDisclose::fax);
    items.insert(Epp::ContactDisclose::email);
    items.insert(Epp::ContactDisclose::vat);
    items.insert(Epp::ContactDisclose::ident);
    items.insert(Epp::ContactDisclose::notify_email);
}

void set_all_disclose_flags(bool to_disclose, Epp::ContactCreateInputData &data)
{
    if (Epp::is_the_default_policy_to_disclose() != to_disclose)
    {
        set_all_items(to_disclose ? data.to_hide : data.to_disclose);
    }
}

void set_correct_contact_data(Epp::ContactCreateInputData &contact_data)
{
    contact_data.name              = "Jan Novak Jr.";
    //contact_data.organization
    contact_data.street1           = "ulice 1";
    contact_data.street2           = "ulice 2";
    contact_data.street3           = "ulice 3";
    contact_data.city              = "mesto";
    contact_data.state_or_province = "hejtmanstvi";
    contact_data.postal_code       = "12345";
    contact_data.country_code      = "CZ";
    contact_data.telephone         = "+420 123 456 789";
    contact_data.fax               = "+420 987 654 321";
    contact_data.email             = "jan@novak.novak";
    contact_data.notify_email      = "jan.notify@novak.novak";
    contact_data.VAT               = "MyVATstring";
    //contact_data.ident
    //contact_data.identtype
    contact_data.authinfo          = "authInfo123";
    set_all_disclose_flags(true, contact_data);
}

template < Epp::ContactDisclose::Enum ITEM >
bool disclose(const Epp::ContactCreateInputData &_data)
{
    const bool item_has_to_be_hidden    = _data.to_hide.find(ITEM) != _data.to_hide.end();
    const bool item_has_to_be_disclosed = _data.to_disclose.find(ITEM) != _data.to_disclose.end();
    if (item_has_to_be_hidden && !item_has_to_be_disclosed) {
        return false;
    }
    if (!item_has_to_be_hidden && item_has_to_be_disclosed) {
        return true;
    }
    if (!item_has_to_be_hidden && !item_has_to_be_disclosed) {
        return Epp::is_the_default_policy_to_disclose();
    }
    throw std::runtime_error("Ambiguous disclose flag");
}

}

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(ContactCreateImpl)

BOOST_FIXTURE_TEST_CASE(create_invalid_registrar_id, has_registrar)
{
    Epp::ContactCreateInputData contact_data;
    set_correct_contact_data(contact_data);

    BOOST_CHECK_THROW(
        Epp::contact_create_impl(
            ctx,
            "contacthandle",
            contact_data,
            0 /* <== !!! */,
            42
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(create_fail_handle_format, has_registrar)
{
    Epp::ContactCreateInputData contact_data;
    set_correct_contact_data(contact_data);

    BOOST_CHECK_THROW(
        Epp::contact_create_impl(
            ctx,
            "contacthandle1?" /* <== !!! */,
            contact_data,
            registrar.id,
            42 /* TODO */
        ),
        Epp::InvalidHandle
    );
}

BOOST_FIXTURE_TEST_CASE(create_fail_already_existing, has_contact)
{
    Epp::ContactCreateInputData contact_data;
    set_correct_contact_data(contact_data);

    BOOST_CHECK_THROW(
        Epp::contact_create_impl(
            ctx,
            contact.handle,
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

    Epp::ContactCreateInputData contact_data;
    set_correct_contact_data(contact_data);

    try {
        Epp::contact_create_impl(
            ctx,
            contact.handle,
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
    Epp::ContactCreateInputData contact_data;
    set_correct_contact_data(contact_data);
    contact_data.country_code      = "1Z9"; /* <- !!! */

    try{
        Epp::contact_create_impl(
            ctx,
            "contacthandle1",
            contact_data,
            registrar.id,
            42 /* TODO */
        );
    } catch (...) {
        Test::check_correct_aggregated_exception_was_thrown(Epp::Error::of_scalar_parameter(Epp::Param::contact_cc, Epp::Reason::country_notexist));
    }
}

void check_equal(const Epp::ContactCreateInputData& create_data, const Fred::InfoContactData& info_data)
{
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

    BOOST_CHECK_EQUAL( create_data.authinfo,                                        info_data.authinfopw );
    BOOST_CHECK_EQUAL( disclose< Epp::ContactDisclose::name         >(create_data), info_data.disclosename );
    BOOST_CHECK_EQUAL( disclose< Epp::ContactDisclose::organization >(create_data), info_data.discloseorganization );
    BOOST_CHECK_EQUAL( disclose< Epp::ContactDisclose::address      >(create_data), info_data.discloseaddress );
    BOOST_CHECK_EQUAL( disclose< Epp::ContactDisclose::telephone    >(create_data), info_data.disclosetelephone );
    BOOST_CHECK_EQUAL( disclose< Epp::ContactDisclose::fax          >(create_data), info_data.disclosefax );
    BOOST_CHECK_EQUAL( disclose< Epp::ContactDisclose::email        >(create_data), info_data.discloseemail );
    BOOST_CHECK_EQUAL( disclose< Epp::ContactDisclose::vat          >(create_data), info_data.disclosevat );
    BOOST_CHECK_EQUAL( disclose< Epp::ContactDisclose::ident        >(create_data), info_data.discloseident );
    BOOST_CHECK_EQUAL( disclose< Epp::ContactDisclose::notify_email >(create_data), info_data.disclosenotifyemail );
}

BOOST_FIXTURE_TEST_CASE(create_ok_all_data, has_registrar)
{
    Epp::ContactCreateInputData contact_data;
    set_correct_contact_data(contact_data);
    const std::string contact_handle = "contacthandle1";

    const Epp::ContactCreateResult result = Epp::contact_create_impl(
        ctx,
        contact_handle,
        contact_data,
        registrar.id,
        42 /* TODO */
    );

    /* check returned data and db changes */
    {
        const Fred::InfoContactData check_sample = Fred::InfoContactByHandle(contact_handle).exec(ctx).info_contact_data;
        BOOST_CHECK_EQUAL( check_sample.id, result.id );
        const Database::Result db_res = ctx.get_conn().exec("SELECT NOW() AT TIME ZONE 'utc'");
        const std::string current_utc_time = static_cast< std::string >(db_res[0][0]);
        BOOST_CHECK_EQUAL(boost::posix_time::time_from_string(current_utc_time), result.crdate);
        BOOST_CHECK_EQUAL(boost::to_upper_copy(contact_handle), check_sample.handle);
        check_equal(contact_data, check_sample);
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
