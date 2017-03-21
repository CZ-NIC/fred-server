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

#include "tests/interfaces/epp/fixture.h"
#include "tests/interfaces/epp/contact/fixture.h"
#include "tests/interfaces/epp/util.h"

#include "src/epp/contact/create_contact.h"
#include "src/epp/contact/create_contact_input_data.h"
#include "src/epp/impl/disclose_policy.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>

namespace Test {

namespace {

template < ::Epp::Contact::ContactDisclose::Item::Enum ITEM>
bool to_disclose(const ::Epp::Contact::CreateContactInputData& _data)
{
    if (!_data.disclose.is_initialized()) {
        return ::Epp::is_the_default_policy_to_disclose();
    }
    return _data.disclose->should_be_disclosed< ITEM >(::Epp::is_the_default_policy_to_disclose());
}

std::string ident_type_to_string(::Epp::Contact::ContactChange::IdentType::Enum type)
{
    switch (type)
    {
        case ::Epp::Contact::ContactChange::IdentType::op:       return Fred::PersonalIdUnion::get_OP("").get_type();
        case ::Epp::Contact::ContactChange::IdentType::pass:     return Fred::PersonalIdUnion::get_PASS("").get_type();
        case ::Epp::Contact::ContactChange::IdentType::ico:      return Fred::PersonalIdUnion::get_ICO("").get_type();
        case ::Epp::Contact::ContactChange::IdentType::mpsv:     return Fred::PersonalIdUnion::get_MPSV("").get_type();
        case ::Epp::Contact::ContactChange::IdentType::birthday: return Fred::PersonalIdUnion::get_BIRTHDAY("").get_type();
    }
    throw std::runtime_error("Invalid ::Epp::Contact::ContactChange::IdentType::Enum value.");
}

void check_equal(const ::Epp::Contact::CreateContactInputData& create_data, const Fred::InfoContactData& info_data)
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
    BOOST_CHECK_EQUAL(create_data.vat,               info_data.vat.get_value_or_default());
    BOOST_CHECK_EQUAL(create_data.ident,             info_data.ssn.get_value_or_default());

    BOOST_CHECK_EQUAL(create_data.identtype.isnull()
        ? ""
        : ident_type_to_string(create_data.identtype.get_value()),
            info_data.ssntype.get_value_or_default());

    BOOST_CHECK_EQUAL(create_data.authinfopw ? *create_data.authinfopw : std::string("not set"), info_data.authinfopw);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::name         >(create_data), info_data.disclosename);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::organization >(create_data), info_data.discloseorganization);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::address      >(create_data), info_data.discloseaddress);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::telephone    >(create_data), info_data.disclosetelephone);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::fax          >(create_data), info_data.disclosefax);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::email        >(create_data), info_data.discloseemail);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::vat          >(create_data), info_data.disclosevat);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::ident        >(create_data), info_data.discloseident);
    BOOST_CHECK_EQUAL(to_disclose< ::Epp::Contact::ContactDisclose::Item::notify_email >(create_data), info_data.disclosenotifyemail);
}

} // namespace Test::{anonymous}

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(CreateContact)

bool create_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_registrar_id, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::create_contact(
                    ctx,
                    DefaultCreateContactInputData().handle,
                    DefaultCreateContactInputData(),
                    DefaultCreateContactConfigData(),
                    session_with_unauthenticated_registrar.data),
            ::Epp::EppResponseFailure,
            create_invalid_registrar_id_exception);
}

bool create_fail_handle_format_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_syntax_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::contact_handle);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::bad_format_contact_handle);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_fail_handle_format, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::create_contact(
                    ctx,
                    InvalidHandle().handle,
                    DefaultCreateContactInputData(),
                    DefaultCreateContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_fail_handle_format_exception);
}

bool create_fail_already_existing_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_exists);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_fail_already_existing, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::create_contact(
                    ctx,
                    contact.data.handle,
                    DefaultCreateContactInputData(),
                    DefaultCreateContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_fail_already_existing_exception);
}

bool create_fail_protected_handle_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_REQUIRE(!e.epp_result().extended_errors()->empty());
    BOOST_CHECK(::Epp::has_extended_error_with_param_reason(e.epp_result(), ::Epp::Param::contact_handle, ::Epp::Reason::protected_period));
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_fail_protected_handle, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    {
        Fred::DeleteContactByHandle(contact.data.handle).exec(ctx);
    }

    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::create_contact(
                    ctx,
                    contact.data.handle,
                    DefaultCreateContactInputData(),
                    DefaultCreateContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_fail_protected_handle_exception);
}

bool create_fail_nonexistent_countrycode_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_REQUIRE(!e.epp_result().extended_errors()->empty());
    BOOST_CHECK(::Epp::has_extended_error_with_param_reason(e.epp_result(), ::Epp::Param::contact_cc, ::Epp::Reason::country_notexist));
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_fail_nonexistent_countrycode, supply_ctx<HasRegistrarWithSession>)
{
    DefaultCreateContactInputData create_contact_input_data;
    create_contact_input_data.country_code = "1Z9";

    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::create_contact(
                    ctx,
                    create_contact_input_data.handle,
                    create_contact_input_data,
                    DefaultCreateContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_fail_nonexistent_countrycode_exception);
}

BOOST_FIXTURE_TEST_CASE(create_ok_all_data, supply_ctx<HasRegistrarWithSession>)
{
    DefaultCreateContactInputData create_contact_input_data;

    const ::Epp::Contact::CreateContactResult result =
            ::Epp::Contact::create_contact(
                    ctx,
                    create_contact_input_data.handle,
                    create_contact_input_data,
                    DefaultCreateContactConfigData(),
                    session.data);

    /* check returned data and db changes */
    {
        const Fred::InfoContactData check_sample = Fred::InfoContactByHandle(create_contact_input_data.handle).exec(ctx).info_contact_data;
        BOOST_CHECK_EQUAL( check_sample.id, result.id );
        const Database::Result db_res = ctx.get_conn().exec("SELECT NOW() AT TIME ZONE 'utc'");
        const std::string current_utc_time = static_cast< std::string >(db_res[0][0]);
        BOOST_CHECK_EQUAL(boost::posix_time::time_from_string(current_utc_time), result.crdate);
        BOOST_CHECK_EQUAL(boost::to_upper_copy(create_contact_input_data.handle), check_sample.handle);
        check_equal(create_contact_input_data, check_sample);
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
