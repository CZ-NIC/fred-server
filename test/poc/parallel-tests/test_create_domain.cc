/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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

#include "test/poc/parallel-tests/fixtures/epp.hh"

#include "src/backend/epp/domain/create_domain.hh"
#include "src/backend/epp/domain/create_domain_localized.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"

#include "libfred/object/generate_authinfo_password.hh"
#include "libfred/object/object_id_handle_pair.hh"

#include <algorithm>
#include <string>
#include <vector>

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(CreateDomain)

BOOST_FIXTURE_TEST_CASE(create_invalid_registrar_id, ::Test::EppFixture<::Test::HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    ::Test::DefaultCreateDomainInputData{},
                    ::Test::DefaultCreateDomainConfigData{},
                    session_with_unauthenticated_registrar.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
                BOOST_CHECK(e.epp_result().empty());
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_invalid_fqdn_zone, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn += "c";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::not_applicable_domain);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_invalid_cz, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = ::Test::CzZone::fqdn();
    BOOST_REQUIRE_EQUAL(create_domain_input_data.data.fqdn, "cz");

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::not_applicable_domain);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_invalid_dot_cz, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = std::string(1, '.') + ::Test::CzZone::fqdn();
    BOOST_REQUIRE_EQUAL(create_domain_input_data.data.fqdn, ".cz");

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::not_applicable_domain);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(fqdn_syntax_2dot_cz, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = std::string(2, '.') + ::Test::CzZone::fqdn();
    BOOST_REQUIRE_EQUAL(create_domain_input_data.data.fqdn, "..cz");

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_syntax_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::bad_format_fqdn);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(fqdn_syntax_front_hyphen, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = "-" + create_domain_input_data.data.fqdn;

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_syntax_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::bad_format_fqdn);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(fqdn_syntax_front_dot, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = "." + create_domain_input_data.data.fqdn;

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_syntax_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::bad_format_fqdn);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(fqdn_syntax_double_dot, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = ::Test::CzZone::fqdn("testdomain1..");
    BOOST_REQUIRE_EQUAL(create_domain_input_data.data.fqdn, "testdomain1..cz");

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_syntax_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::bad_format_fqdn);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(fqdn_syntax_no_dot, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = "testdomain1";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::not_applicable_domain);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_fail_already_existing, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    BOOST_CHECK_NO_THROW(::Test::TheSameDomain(ctx, create_domain_input_data.data, registrar));

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_exists);
                BOOST_CHECK(e.epp_result().empty());
                //print_epp_response_failure(e);
                return true;
            });
}

using CreateFqdnBlacklistedFixture = ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData, ::Test::HasBlacklistedCzFqdn>;
BOOST_FIXTURE_TEST_CASE(create_fqdn_blacklisted, CreateFqdnBlacklistedFixture)
{
    create_domain_input_data.data.fqdn = blacklisted.fqdn;
    BOOST_REQUIRE_EQUAL(create_domain_input_data.data.fqdn, "blacklistedfqdn.cz");

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::blacklisted_domain);
                //print_epp_response_failure(e);
                return true;
            });
}

using CreateFailRegistrarZoneAccessFixture = ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData, ::Test::HasRegistrarNotInZone>;
BOOST_FIXTURE_TEST_CASE(create_fail_registrar_zone_access, CreateFailRegistrarZoneAccessFixture)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    ::Test::Session{registrar_not_in_zone.data.id}.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authorization_error);
                BOOST_CHECK(e.epp_result().empty());
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_invalid_nsset, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.nsset = create_domain_input_data.data.nsset + "NONEXISTING";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.nsset " << create_domain_input_data.data.nsset);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_nsset);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::nsset_notexist);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_invalid_keyset, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.keyset = create_domain_input_data.data.keyset + "NONEXISTING";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.keyset " << create_domain_input_data.data.keyset);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_keyset);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::keyset_notexist);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_empty_registrant, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.registrant = "";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.registrant " << create_domain_input_data.data.registrant);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_registrant);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::registrant_notexist);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_invalid_registrant, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.registrant = create_domain_input_data.data.registrant + "NONEXISTING";

    BOOST_TEST_MESSAGE("create_domain_input_data.data.registrant " << create_domain_input_data.data.registrant);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_registrant);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::registrant_notexist);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_invalid_period_negative, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.period = ::Epp::Domain::DomainRegistrationTime(-12, ::Epp::Domain::DomainRegistrationTime::Unit::month);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_period);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::period_range);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_invalid_period_toolong, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.period = ::Epp::Domain::DomainRegistrationTime(132, ::Epp::Domain::DomainRegistrationTime::Unit::month);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_period);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::period_range);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_invalid_period_modulo, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.period = ::Epp::Domain::DomainRegistrationTime(25, ::Epp::Domain::DomainRegistrationTime::Unit::month);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_period);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::period_policy);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_empty_valexdate_enum, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = ::Test::CzEnumZone::fqdn(222547111);
    BOOST_REQUIRE_EQUAL(create_domain_input_data.data.fqdn, "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa");
    create_domain_input_data.data.enum_validation_extension = boost::none;

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::required_parameter_missing);
                BOOST_CHECK(e.epp_result().empty());
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_special_valexdate_enum, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.fqdn = ::Test::CzEnumZone::fqdn(222547111);
    BOOST_REQUIRE_EQUAL(create_domain_input_data.data.fqdn, "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa");
    create_domain_input_data.data.enum_validation_extension = ::Epp::Domain::EnumValidationExtension{};

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_ext_val_date);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::valexpdate_not_valid);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_nonempty_valexdate_nonenum, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.enum_validation_extension = ::Epp::Domain::EnumValidationExtension{};

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_ext_val_date);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::valexpdate_not_used);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_enum_valexdate_today, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    const auto current_local_date = Test::get_current_local_date(ctx);

    create_domain_input_data.data.fqdn = ::Test::CzEnumZone::fqdn(222547111);
    BOOST_REQUIRE_EQUAL(create_domain_input_data.data.fqdn, "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa");
    create_domain_input_data.data.enum_validation_extension = ::Epp::Domain::EnumValidationExtension{current_local_date, false}; //today

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_ext_val_date);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::valexpdate_not_valid);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_enum_valexdate_yesterday, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    const auto current_local_date = Test::get_current_local_date(ctx);

    create_domain_input_data.data.fqdn = ::Test::CzEnumZone::fqdn(222547111);
    BOOST_REQUIRE_EQUAL(create_domain_input_data.data.fqdn, "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa");
    create_domain_input_data.data.enum_validation_extension = ::Epp::Domain::EnumValidationExtension{current_local_date - boost::gregorian::days{1}, false}; //yesterday

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_ext_val_date);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::valexpdate_not_valid);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_enum_valexdate_7m, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    const auto current_local_date = Test::get_current_local_date(ctx);

    create_domain_input_data.data.fqdn = ::Test::CzEnumZone::fqdn(222547111);
    BOOST_REQUIRE_EQUAL(create_domain_input_data.data.fqdn, "1.1.1.7.4.5.2.2.2.0.2.4.e164.arpa");
    create_domain_input_data.data.enum_validation_extension = ::Epp::Domain::EnumValidationExtension{current_local_date + boost::gregorian::months{7}, false}; //7 months

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_range_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_ext_val_date);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::valexpdate_not_valid);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_nonexistent_admin, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.admin_contacts.push_back(create_domain_input_data.data.admin_contacts.back() + "NONEXISTING");

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_admin);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 3);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::admin_notexist);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_duplicated_admin, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    create_domain_input_data.data.admin_contacts.push_back(create_domain_input_data.data.admin_contacts.back());

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_policy_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_admin);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 3);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::duplicated_contact);
                //print_epp_response_failure(e);
                return true;
            });
}

using CreateInvalidDomainBySystemRegistrarSuccessFixture = ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData, ::Test::HasSystemRegistrar>;
BOOST_FIXTURE_TEST_CASE(create_invalid_domain_by_system_registrar_success, CreateInvalidDomainBySystemRegistrarSuccessFixture)
{
    create_domain_input_data.data.fqdn = ::Test::CzZone::fqdn("xn--j--ra-xqa"); // já--ra.cz
    BOOST_REQUIRE_EQUAL(create_domain_input_data.data.fqdn, "xn--j--ra-xqa.cz");
    BOOST_REQUIRE(system_registrar.data.system.get_value());

    BOOST_TEST_MESSAGE("create_domain_input_data.data.fqdn " << create_domain_input_data.data.fqdn);

    BOOST_REQUIRE_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](auto&& e)
            {
                BOOST_TEST_MESSAGE("EppResponseFailure: " << e.what());
                return true;
            });

    BOOST_CHECK_NO_THROW(::Epp::Domain::create_domain(
            ctx,
            create_domain_input_data.data,
            ::Test::DefaultCreateDomainConfigData{},
            ::Test::Session{system_registrar.data.id}.data));
}

using CreateInvalidDomainBySystemRegistrarFailFixture = ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData, ::Test::HasSystemRegistrar>;
BOOST_FIXTURE_TEST_CASE(create_invalid_domain_by_system_registrar_fail, CreateInvalidDomainBySystemRegistrarFailFixture)
{
    create_domain_input_data.data.fqdn = "-" + create_domain_input_data.data.fqdn;

    BOOST_TEST_MESSAGE(std::string("create_domain_input_data.data.fqdn ") << create_domain_input_data.data.fqdn);

    BOOST_REQUIRE_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data),
            ::Epp::EppResponseFailure,
            [](auto&& e)
            {
                BOOST_TEST_MESSAGE("EppResponseFailure: " << e.what());
                return true;
            });

    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    ::Test::Session{system_registrar.data.id}.data),
            ::Epp::EppResponseFailure,
            [](const ::Epp::EppResponseFailure& e)
            {
                BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::parameter_value_syntax_error);
                BOOST_REQUIRE(e.epp_result().extended_errors());
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::domain_fqdn);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
                BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::bad_format_fqdn);
                //print_epp_response_failure(e);
                return true;
            });
}

BOOST_FIXTURE_TEST_CASE(create_ok, ::Test::EppFixture<::Test::HasRegistrarWithSessionAndCreateDomainInputData>)
{
    BOOST_REQUIRE_NO_THROW(
            ::Epp::Domain::create_domain(
                    ctx,
                    create_domain_input_data.data,
                    ::Test::DefaultCreateDomainConfigData{},
                    session.data));

    const auto info_data = ::LibFred::InfoDomainByFqdn{create_domain_input_data.data.fqdn}.exec(ctx, "UTC").info_domain_data;
    BOOST_TEST_MESSAGE(info_data.to_string());

    const auto expected_expiration_date_local = Test::get_current_local_date(ctx) + boost::gregorian::years{1};

    BOOST_CHECK_EQUAL(info_data.fqdn, create_domain_input_data.data.fqdn);
    BOOST_CHECK_EQUAL(info_data.registrant.handle, create_domain_input_data.data.registrant);
    BOOST_CHECK_EQUAL(info_data.nsset.get_value().handle, create_domain_input_data.data.nsset);
    BOOST_CHECK_EQUAL(info_data.keyset.get_value().handle, create_domain_input_data.data.keyset);
    BOOST_TEST_MESSAGE("info_data.expiration_date: " << info_data.expiration_date << " expected_expiration_date_local: " << expected_expiration_date_local);
    BOOST_CHECK_EQUAL(info_data.expiration_date, expected_expiration_date_local);

    BOOST_TEST_MESSAGE("info_data.admin_contacts.size(): " << info_data.admin_contacts.size());
    BOOST_REQUIRE_LE(2, info_data.admin_contacts.size());
    BOOST_TEST_MESSAGE("create_domain_input_data.data.admin_contacts " + create_domain_input_data.data.admin_contacts.at(0));
    BOOST_TEST_MESSAGE("create_domain_input_data.data.admin_contacts " + create_domain_input_data.data.admin_contacts.at(1));

    BOOST_CHECK_EQUAL(info_data.admin_contacts.size(), create_domain_input_data.data.admin_contacts.size());
    BOOST_REQUIRE_LE(create_domain_input_data.data.admin_contacts.size(), info_data.admin_contacts.size());
    BOOST_CHECK(std::equal(create_domain_input_data.data.admin_contacts.begin(), create_domain_input_data.data.admin_contacts.end(), info_data.admin_contacts.begin(), [](auto&& handle, auto&& pair) { return handle == pair.handle; }));

    BOOST_CHECK(info_data.enum_domain_validation.isnull());
}

BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Domain/CreateDomain
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Domain
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp
BOOST_AUTO_TEST_SUITE_END()//Backend
