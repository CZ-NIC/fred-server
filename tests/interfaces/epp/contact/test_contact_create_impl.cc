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

#include "src/epp/impl/disclose_policy.h"
#include "src/epp/contact/create_contact.h"

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>

namespace {

boost::optional< Epp::ContactDisclose > set_all_disclose_flags(bool to_disclose)
{
    if (Epp::is_the_default_policy_to_disclose() == to_disclose)
    {
        return boost::optional< Epp::ContactDisclose >();
    }
    Epp::ContactDisclose disclose(to_disclose ? Epp::ContactDisclose::Flag::hide
                                              : Epp::ContactDisclose::Flag::disclose);
    disclose.add< Epp::ContactDisclose::Item::name >();
    disclose.add< Epp::ContactDisclose::Item::organization >();
    disclose.add< Epp::ContactDisclose::Item::address >();
    disclose.add< Epp::ContactDisclose::Item::telephone >();
    disclose.add< Epp::ContactDisclose::Item::fax >();
    disclose.add< Epp::ContactDisclose::Item::email >();
    disclose.add< Epp::ContactDisclose::Item::vat >();
    disclose.add< Epp::ContactDisclose::Item::ident >();
    disclose.add< Epp::ContactDisclose::Item::notify_email >();
    return disclose;
}

void set_correct_contact_data(Epp::ContactChange &contact_data)
{
    contact_data.name              = "Jan Novak Jr.";
    contact_data.organization      = "";
    contact_data.streets.clear();
    contact_data.streets.reserve(3);
    contact_data.streets.push_back(Nullable< std::string >("ulice 1"));
    contact_data.streets.push_back(Nullable< std::string >("ulice 2"));
    contact_data.streets.push_back(Nullable< std::string >("ulice 3"));
    contact_data.city              = "mesto";
    contact_data.state_or_province = "hejtmanstvi";
    contact_data.postal_code       = "12345";
    contact_data.country_code      = "CZ";
    contact_data.telephone         = "+420 123 456 789";
    contact_data.fax               = "+420 987 654 321";
    contact_data.email             = "jan@novak.novak";
    contact_data.notify_email      = "jan.notify@novak.novak";
    contact_data.vat               = "MyVATstring";
    contact_data.ident             = "";
    contact_data.ident_type        = Nullable< Epp::ContactChange::IdentType::Enum >();
    contact_data.auth_info_pw      = "authInfo123";
    contact_data.disclose          = set_all_disclose_flags(true);
}

template < Epp::ContactDisclose::Item::Enum ITEM >
bool to_disclose(const Epp::ContactCreateInputData &_data)
{
    if (!_data.disclose.is_initialized()) {
        return Epp::is_the_default_policy_to_disclose();
    }
    return _data.disclose->should_be_disclosed< ITEM >(Epp::is_the_default_policy_to_disclose());
}

std::string ident_type_to_string(Epp::ContactChange::IdentType::Enum type)
{
    switch (type)
    {
        case Epp::ContactChange::IdentType::op:       return Fred::PersonalIdUnion::get_OP("").get_type();
        case Epp::ContactChange::IdentType::pass:     return Fred::PersonalIdUnion::get_PASS("").get_type();
        case Epp::ContactChange::IdentType::ico:      return Fred::PersonalIdUnion::get_ICO("").get_type();
        case Epp::ContactChange::IdentType::mpsv:     return Fred::PersonalIdUnion::get_MPSV("").get_type();
        case Epp::ContactChange::IdentType::birthday: return Fred::PersonalIdUnion::get_BIRTHDAY("").get_type();
    }
    throw std::runtime_error("Invalid Epp::ContactChange::IdentType::Enum value.");
}

void check_equal(const Epp::ContactCreateInputData &create_data, const Fred::InfoContactData &info_data)
{
    BOOST_CHECK_EQUAL(create_data.name,              info_data.name.get_value_or_default() );
    BOOST_CHECK_EQUAL(create_data.organization,      info_data.organization.get_value_or_default() );

    BOOST_CHECK_EQUAL(0 < create_data.streets.size() ? create_data.streets[0] : "",
                      info_data.place.get_value_or_default().street1);
    BOOST_CHECK_EQUAL(1 < create_data.streets.size() ? create_data.streets[1] : "",
                      info_data.place.get_value_or_default().street2.get_value_or_default());
    BOOST_CHECK_EQUAL(2 < create_data.streets.size() ? create_data.streets[2] : "",
                      info_data.place.get_value_or_default().street3.get_value_or_default());
    BOOST_CHECK_EQUAL(create_data.city,              info_data.place.get_value_or_default().city);
    BOOST_CHECK_EQUAL(create_data.postal_code,       info_data.place.get_value_or_default().postalcode);
    BOOST_CHECK_EQUAL(create_data.state_or_province, info_data.place.get_value_or_default().stateorprovince.get_value_or_default());
    BOOST_CHECK_EQUAL(create_data.country_code,      info_data.place.get_value_or_default().country);
    BOOST_CHECK_EQUAL(create_data.telephone,         info_data.telephone.get_value_or_default());
    BOOST_CHECK_EQUAL(create_data.fax,               info_data.fax.get_value_or_default());
    BOOST_CHECK_EQUAL(create_data.email,             info_data.email.get_value_or_default());
    BOOST_CHECK_EQUAL(create_data.notify_email,      info_data.notifyemail.get_value_or_default());
    BOOST_CHECK_EQUAL(create_data.VAT,               info_data.vat.get_value_or_default());
    BOOST_CHECK_EQUAL(create_data.ident,             info_data.ssn.get_value_or_default());

    BOOST_CHECK_EQUAL(create_data.identtype.isnull()
        ? ""
        : ident_type_to_string(create_data.identtype.get_value()),
            info_data.ssntype.get_value_or_default());

    BOOST_CHECK_EQUAL(create_data.authinfo ? *create_data.authinfo : std::string("not set"), info_data.authinfopw);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::name         >(create_data), info_data.disclosename);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::organization >(create_data), info_data.discloseorganization);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::address      >(create_data), info_data.discloseaddress);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::telephone    >(create_data), info_data.disclosetelephone);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::fax          >(create_data), info_data.disclosefax);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::email        >(create_data), info_data.discloseemail);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::vat          >(create_data), info_data.disclosevat);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::ident        >(create_data), info_data.discloseident);
    BOOST_CHECK_EQUAL(to_disclose< Epp::ContactDisclose::Item::notify_email >(create_data), info_data.disclosenotifyemail);
}

}

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(ContactCreateImpl)

BOOST_FIXTURE_TEST_CASE(create_invalid_registrar_id, has_registrar)
{
    Epp::ContactChange contact_data;
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
    Epp::ContactChange contact_data;
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
    Epp::ContactChange contact_data;
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

    Epp::ContactChange contact_data;
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
    Epp::ContactChange contact_data;
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

BOOST_FIXTURE_TEST_CASE(create_ok_all_data, has_registrar)
{
    Epp::ContactChange contact_data;
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
