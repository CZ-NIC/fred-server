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
#include "tests/interfaces/epp/domain/fixture.h"
#include "tests/interfaces/epp/util.h"

#include "src/epp/domain/delete_domain.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/param.h"
#include "src/epp/reason.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(DeleteDomain)

bool fail_invalid_registrar_id_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_invalid_registrar_id, Test::supply_ctx<Test::Fixture::HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            Epp::Domain::delete_domain(
                    ctx,
                    "domain.cz",
                    Test::DefaultDeleteDomainConfigData(),
                    session_with_unauthenticated_registrar.data),
            Epp::EppResponseFailure,
            fail_invalid_registrar_id_exception);
}

bool fail_nonexistent_handle_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_domain_does_not_exist, Test::supply_ctx<Test::Fixture::HasRegistrarWithSessionAndNonexistentFqdn>)
{
    BOOST_CHECK_EXCEPTION(
            Epp::Domain::delete_domain(
                    ctx,
                    nonexistent_fqdn.fqdn,
                    Test::DefaultDeleteDomainConfigData(),
                    session.data),
            Epp::EppResponseFailure,
            fail_nonexistent_handle_exception);
}

bool fail_enum_domain_does_not_exist_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_enum_domain_does_not_exist, Test::supply_ctx<Test::Fixture::HasRegistrarWithSessionAndNonexistentEnumFqdn>)
{
    BOOST_CHECK_EXCEPTION(
            Epp::Domain::delete_domain(
                    ctx,
                    nonexistent_enum_fqdn.fqdn,
                    Test::DefaultDeleteDomainConfigData(),
                    session.data),
            Epp::EppResponseFailure,
            fail_enum_domain_does_not_exist_exception);
}

bool fail_wrong_registrar_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authorization_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::registrar_autor);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::unauthorized_registrar);
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_wrong_registrar, Test::supply_ctx<Test::Fixture::HasRegistrarWithSessionAndDomainOfDifferentRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            Epp::Domain::delete_domain(
                    ctx,
                    domain_of_different_registrar.data.fqdn,
                    Test::DefaultDeleteDomainConfigData(),
                    session.data),
            Epp::EppResponseFailure,
            fail_wrong_registrar_exception);
}

bool fail_registrar_without_zone_access_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authorization_error);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_registrar_without_zone_access, Test::supply_ctx<Test::Fixture::HasRegistrarWithSessionAndDomain>)
{
    BOOST_CHECK_EXCEPTION(
            Epp::Domain::delete_domain(
                    ctx,
                    domain.data.fqdn,
                    Test::DefaultDeleteDomainConfigData(),
                    Test::Session(ctx, Test::RegistrarNotInZone(ctx).data.id).data),
            Epp::EppResponseFailure,
            fail_registrar_without_zone_access_exception);
}

bool fail_prohibiting_status_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_prohibiting_status, Test::supply_ctx<Test::Fixture::HasRegistrarWithSession>)
{
    BOOST_CHECK_EXCEPTION(
            Epp::Domain::delete_domain(
                    ctx,
                    Test::DomainWithServerUpdateProhibited(ctx, registrar.data.handle).data.fqdn,
                    Test::DefaultDeleteDomainConfigData(),
                    session.data),
            Epp::EppResponseFailure,
            fail_prohibiting_status_exception);
}

BOOST_FIXTURE_TEST_CASE(ok, Test::supply_ctx<Test::Fixture::HasRegistrarWithSessionAndDomain>)
{
    Epp::Domain::delete_domain(
        ctx,
        domain.data.fqdn,
        Test::DefaultDeleteDomainConfigData(),
        session.data
    );

    BOOST_CHECK_EQUAL(
        Fred::InfoDomainHistoryById(domain.data.id).exec(ctx).rbegin()->info_domain_data.delete_time.isnull(),
        false
    );
}

BOOST_FIXTURE_TEST_CASE(ok_states_are_upgraded, Test::supply_ctx<Test::Fixture::HasRegistrarWithSession>)
{
    Test::DomainWithServerTransferProhibited domain(ctx, registrar.data.handle);

    Epp::Domain::delete_domain(
        ctx,
        domain.data.fqdn,
        Test::DefaultDeleteDomainConfigData(),
        session.data
    );

    // now object has the state server_transfer_prohibited request itself
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(domain.data.id).exec(ctx) ) {
                object_states_after.push_back(state.state_name);
            }
        }

        BOOST_CHECK(
            std::find(object_states_after.begin(), object_states_after.end(), domain.status)
            !=
            object_states_after.end()
        );
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
