/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "test/backend/epp/fixture.hh"
#include "test/backend/epp/domain/fixture.hh"
#include "test/backend/epp/util.hh"

#include "src/backend/epp/domain/create_domain.hh"
#include "src/backend/epp/domain/create_domain_localized.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "libfred/object/generate_authinfo_password.hh"
#include "libfred/object/object_id_handle_pair.hh"

#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(CreateDomain)

bool handle_contact_reference_predicate (const std::string& handle, const ::LibFred::RegistrableObject::Contact::ContactReference& pair)
{
  return (handle == pair.handle);
}

bool create_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_registrar_id, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    DefaultCreateDomainInputData(),
                    DefaultCreateDomainConfigData(),
                    session_with_unauthenticated_registrar.data),
            ::Epp::EppResponseFailure,
            create_invalid_registrar_id_exception);
}

bool create_invalid_fqdn_zone_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::not_applicable_domain);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_fqdn_zone, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn += "c";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_invalid_fqdn_zone_exception);
}

bool create_invalid_cz_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::not_applicable_domain);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_cz, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = "cz";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_invalid_cz_exception);
}

bool create_invalid_dot_cz_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::not_applicable_domain);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_dot_cz, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = ".cz";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_invalid_dot_cz_exception);
}

bool fqdn_syntax_2dot_cz_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_syntax_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::bad_format_fqdn);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(fqdn_syntax_2dot_cz, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = "..cz";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fqdn_syntax_2dot_cz_exception);
}

bool fqdn_syntax_front_hyphen_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_syntax_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::bad_format_fqdn);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(fqdn_syntax_front_hyphen, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = "-" + create_domain_input_data.data.fqdn;

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fqdn_syntax_front_hyphen_exception);
}

bool fqdn_syntax_front_dot_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_syntax_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::bad_format_fqdn);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(fqdn_syntax_front_dot, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = "." + create_domain_input_data.data.fqdn;

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fqdn_syntax_front_dot_exception);
}

bool fqdn_syntax_double_dot_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_syntax_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::bad_format_fqdn);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(fqdn_syntax_double_dot, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = "testdomain1..cz";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fqdn_syntax_double_dot_exception);
}

bool fqdn_syntax_no_dot_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::not_applicable_domain);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(fqdn_syntax_no_dot, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = "testdomain1";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fqdn_syntax_no_dot_exception);
}

bool create_fail_already_existing_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_exists);
    BOOST_CHECK(e.epp_result().empty());
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_fail_already_existing, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    const Domain domain(ctx, registrar.data.handle, create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_fail_already_existing_exception);
}

bool create_fqdn_blacklisted_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::blacklisted_domain);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_fqdn_blacklisted, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = BlacklistedFqdn(ctx).fqdn;

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_fqdn_blacklisted_exception);
}

bool create_fail_registrar_zone_access_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authorization_error);
    BOOST_CHECK(e.epp_result().empty());
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_fail_registrar_zone_access, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    Session(ctx, RegistrarNotInZone(ctx).data.id).data),
            ::Epp::EppResponseFailure,
            create_fail_registrar_zone_access_exception);
}

bool create_invalid_nsset_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_nsset);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::nsset_notexist);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_nsset, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.nsset = create_domain_input_data.data.nsset + "NONEXISTING";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.nsset " << create_domain_input_data.data.nsset);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_invalid_nsset_exception);
}

bool create_invalid_keyset_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_keyset);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::keyset_notexist);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_keyset, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.keyset = create_domain_input_data.data.keyset + "NONEXISTING";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.keyset " << create_domain_input_data.data.keyset);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_invalid_keyset_exception);
}

bool create_empty_registrant_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_registrant);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::registrant_notexist);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_empty_registrant, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.registrant = "";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.registrant " << create_domain_input_data.data.registrant);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_empty_registrant_exception);
}

bool create_invalid_registrant_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_registrant);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::registrant_notexist);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_registrant, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.registrant = create_domain_input_data.data.registrant + "NONEXISTING";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.registrant " << create_domain_input_data.data.registrant);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_invalid_registrant_exception);
}

bool create_invalid_period_negative_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_period);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::period_range);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_period_negative, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.period = ::Epp::Domain::DomainRegistrationTime(-12, ::Epp::Domain::DomainRegistrationTime::Unit::month);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_invalid_period_negative_exception);
}

bool create_invalid_period_toolong_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_period);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::period_range);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_period_toolong, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.period = ::Epp::Domain::DomainRegistrationTime(132, ::Epp::Domain::DomainRegistrationTime::Unit::month);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_invalid_period_toolong_exception);
}

bool create_invalid_period_modulo_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_period);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::period_policy);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_period_modulo, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.period = ::Epp::Domain::DomainRegistrationTime(25, ::Epp::Domain::DomainRegistrationTime::Unit::month);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_invalid_period_modulo_exception);
}

bool create_empty_valexdate_enum_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::required_parameter_missing);
    BOOST_CHECK(e.epp_result().empty());
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_empty_valexdate_enum, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    create_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>();

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_empty_valexdate_enum_exception);
}

bool create_special_valexdate_enum_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_ext_val_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::valexpdate_not_valid);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_special_valexdate_enum, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    create_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
            ::Epp::Domain::EnumValidationExtension());

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_special_valexdate_enum_exception);
}

bool create_nonempty_valexdate_nonenum_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_ext_val_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::valexpdate_not_used);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_nonempty_valexdate_nonenum, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
            ::Epp::Domain::EnumValidationExtension());

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_nonempty_valexdate_nonenum_exception);
}

bool create_enum_valexdate_today_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_ext_val_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::valexpdate_not_valid);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_enum_valexdate_today, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    create_domain_input_data.data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    create_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
            ::Epp::Domain::EnumValidationExtension(current_local_date, false));//yesterday

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_enum_valexdate_today_exception);
}

bool create_enum_valexdate_yesterday_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_ext_val_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::valexpdate_not_valid);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_enum_valexdate_yesterday, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    create_domain_input_data.data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    create_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
            ::Epp::Domain::EnumValidationExtension(current_local_date - boost::gregorian::days(1), false));//yesterday

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_enum_valexdate_yesterday_exception);
}

bool create_enum_valexdate_7m_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_ext_val_date);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::valexpdate_not_valid);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_enum_valexdate_7m, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    const boost::posix_time::ptime current_utc_time = boost::posix_time::time_from_string(
    static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const boost::posix_time::ptime current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    const boost::gregorian::date current_local_date = current_local_time.date();

    create_domain_input_data.data.fqdn = "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa";
    create_domain_input_data.data.enum_validation_extension = boost::optional< ::Epp::Domain::EnumValidationExtension>(
            ::Epp::Domain::EnumValidationExtension(current_local_date + boost::gregorian::months(7), false));//7 months

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_enum_valexdate_7m_exception);
}

bool create_nonexistent_admin_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_admin);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 3);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::admin_notexist);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_nonexistent_admin, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.admin_contacts.push_back(*create_domain_input_data.data.admin_contacts.rbegin() + "NONEXISTING");

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_nonexistent_admin_exception);
}

bool create_duplicated_admin_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_admin);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 3);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::duplicated_contact);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_duplicated_admin, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.admin_contacts.push_back(*create_domain_input_data.data.admin_contacts.rbegin());

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            create_duplicated_admin_exception);
}

BOOST_FIXTURE_TEST_CASE(create_empty_authinfopw, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.authinfopw = boost::optional<std::string>("");

    ::Epp::Domain::create_domain(
            ctx,
            create_domain_input_data.data,
            DefaultCreateDomainConfigData(),
            session.data);

    ::LibFred::InfoDomainData info_data = ::LibFred::InfoDomainByFqdn(create_domain_input_data.data.fqdn).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());

    //warning: timestamp conversion using local system timezone
    const boost::gregorian::date current_local_date = boost::date_time::c_local_adjustor<ptime>::utc_to_local(
        boost::posix_time::time_from_string(static_cast<std::string>(ctx.get_conn().exec(
            "SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]))).date();

    const boost::gregorian::date expected_expiration_date_local = boost::gregorian::from_simple_string(
        static_cast<std::string>(ctx.get_conn().exec_params("select ($1::date + '1 year'::interval)::date",
                Database::query_param_list(current_local_date))[0][0]));

    BOOST_CHECK(info_data.fqdn == create_domain_input_data.data.fqdn);
    BOOST_CHECK(info_data.registrant.handle == create_domain_input_data.data.registrant);
    BOOST_CHECK(info_data.nsset.get_value().handle == create_domain_input_data.data.nsset);
    BOOST_CHECK(info_data.keyset.get_value().handle == create_domain_input_data.data.keyset);
    BOOST_CHECK(info_data.authinfopw.length() == 8);
    BOOST_CHECK(info_data.authinfopw.find_first_not_of(::LibFred::get_chars_allowed_in_generated_authinfopw()) == std::string::npos);
    BOOST_CHECK(info_data.expiration_date == expected_expiration_date_local);

    BOOST_TEST_MESSAGE("info_data.admin_contacts.size(): "<< info_data.admin_contacts.size());

    BOOST_TEST_MESSAGE("create_domain_input_data.data.admin_contacts " + create_domain_input_data.data.admin_contacts.at(0));
    BOOST_TEST_MESSAGE("create_domain_input_data.data.admin_contacts " + create_domain_input_data.data.admin_contacts.at(1));

    BOOST_CHECK(info_data.admin_contacts.size() == create_domain_input_data.data.admin_contacts.size());
    BOOST_CHECK(std::equal (create_domain_input_data.data.admin_contacts.begin(), create_domain_input_data.data.admin_contacts.end(),
            info_data.admin_contacts.begin(), handle_contact_reference_predicate));

    BOOST_CHECK(info_data.enum_domain_validation.isnull());
}

BOOST_FIXTURE_TEST_CASE(create_authinfopw_not_set, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.authinfopw = boost::optional<std::string>();

    ::Epp::Domain::create_domain(
            ctx,
            create_domain_input_data.data,
            DefaultCreateDomainConfigData(),
            session.data);

    ::LibFred::InfoDomainData info_data = ::LibFred::InfoDomainByFqdn(create_domain_input_data.data.fqdn).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());

    //warning: timestamp conversion using local system timezone
    const boost::gregorian::date current_local_date = boost::date_time::c_local_adjustor<ptime>::utc_to_local(
        boost::posix_time::time_from_string(static_cast<std::string>(ctx.get_conn().exec(
            "SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]))).date();

    const boost::gregorian::date expected_expiration_date_local = boost::gregorian::from_simple_string(
        static_cast<std::string>(ctx.get_conn().exec_params("select ($1::date + '1 year'::interval)::date",
                Database::query_param_list(current_local_date))[0][0]));

    BOOST_CHECK(info_data.fqdn == create_domain_input_data.data.fqdn);
    BOOST_CHECK(info_data.registrant.handle == create_domain_input_data.data.registrant);
    BOOST_CHECK(info_data.nsset.get_value().handle == create_domain_input_data.data.nsset);
    BOOST_CHECK(info_data.keyset.get_value().handle == create_domain_input_data.data.keyset);
    BOOST_CHECK(info_data.authinfopw.length() == 8);
    BOOST_CHECK(info_data.authinfopw.find_first_not_of(::LibFred::get_chars_allowed_in_generated_authinfopw()) == std::string::npos);
    BOOST_CHECK(info_data.expiration_date == expected_expiration_date_local);

    BOOST_TEST_MESSAGE("info_data.admin_contacts.size(): "<< info_data.admin_contacts.size());

    BOOST_TEST_MESSAGE("create_domain_input_data.data.admin_contacts " + create_domain_input_data.data.admin_contacts.at(0));
    BOOST_TEST_MESSAGE("create_domain_input_data.data.admin_contacts " + create_domain_input_data.data.admin_contacts.at(1));

    BOOST_CHECK(info_data.admin_contacts.size() == create_domain_input_data.data.admin_contacts.size());
    BOOST_CHECK(std::equal (create_domain_input_data.data.admin_contacts.begin(), create_domain_input_data.data.admin_contacts.end(),
            info_data.admin_contacts.begin(), handle_contact_reference_predicate));

    BOOST_CHECK(info_data.enum_domain_validation.isnull());
}

BOOST_FIXTURE_TEST_CASE(create_invalid_domain_by_system_registrar_success, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = std::string("xn--j--ra-xqa.cz"); // já--ra.cz

    BOOST_TEST_MESSAGE(std::string("create_domain_input_data.data.fqdn ") << create_domain_input_data.data.fqdn);

    ::Epp::Domain::create_domain(
            ctx,
            create_domain_input_data.data,
            DefaultCreateDomainConfigData(),
            Session(ctx, SystemRegistrar(ctx).data.id).data);
}

bool create_invalid_domain_by_system_registrar_fail_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_syntax_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::bad_format_fqdn);
    //print_epp_response_failure(e);
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_domain_by_system_registrar_fail, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = "-" + create_domain_input_data.data.fqdn;

    BOOST_TEST_MESSAGE(std::string("create_domain_input_data.data.fqdn ") << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    Session(ctx, SystemRegistrar(ctx).data.id).data),
            ::Epp::EppResponseFailure,
            create_invalid_domain_by_system_registrar_fail_exception);
}

BOOST_FIXTURE_TEST_CASE(create_ok, supply_ctx<HasRegistrarWithSessionAndCreateDomainInputData>)
{
    BOOST_CHECK_NO_THROW(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    DefaultCreateDomainConfigData(),
                    session.data));

    ::LibFred::InfoDomainData info_data = ::LibFred::InfoDomainByFqdn(create_domain_input_data.data.fqdn).exec(ctx,"UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());


    //warning: timestamp conversion using local system timezone
    const boost::gregorian::date current_local_date = boost::date_time::c_local_adjustor<ptime>::utc_to_local(
        boost::posix_time::time_from_string(static_cast<std::string>(ctx.get_conn().exec(
            "SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]))).date();

    const boost::gregorian::date expected_expiration_date_local = boost::gregorian::from_simple_string(
        static_cast<std::string>(ctx.get_conn().exec_params("select ($1::date + '1 year'::interval)::date",
                Database::query_param_list(current_local_date))[0][0]));

    BOOST_CHECK(info_data.fqdn == create_domain_input_data.data.fqdn);
    BOOST_CHECK(info_data.registrant.handle == create_domain_input_data.data.registrant);
    BOOST_CHECK(info_data.nsset.get_value().handle == create_domain_input_data.data.nsset);
    BOOST_CHECK(info_data.keyset.get_value().handle == create_domain_input_data.data.keyset);
    BOOST_CHECK(info_data.authinfopw == create_domain_input_data.data.authinfopw);
    BOOST_TEST_MESSAGE("info_data.expiration_date: " << info_data.expiration_date << " expected_expiration_date_local: " << expected_expiration_date_local);
    BOOST_CHECK(info_data.expiration_date == expected_expiration_date_local);

    BOOST_TEST_MESSAGE("info_data.admin_contacts.size(): "<< info_data.admin_contacts.size());

    BOOST_TEST_MESSAGE("create_domain_input_data.data.admin_contacts " + create_domain_input_data.data.admin_contacts.at(0));
    BOOST_TEST_MESSAGE("create_domain_input_data.data.admin_contacts " + create_domain_input_data.data.admin_contacts.at(1));

    BOOST_CHECK(info_data.admin_contacts.size() == create_domain_input_data.data.admin_contacts.size());
    BOOST_CHECK(std::equal (create_domain_input_data.data.admin_contacts.begin(), create_domain_input_data.data.admin_contacts.end(),
            info_data.admin_contacts.begin(), handle_contact_reference_predicate));

    BOOST_CHECK(info_data.enum_domain_validation.isnull());

}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
