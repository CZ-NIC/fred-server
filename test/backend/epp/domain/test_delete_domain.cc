/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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

#include "src/backend/epp/domain/delete_domain.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/param.hh"
#include "src/backend/epp/reason.hh"
#include "src/backend/epp/poll/poll_request.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/variant/get.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(DeleteDomain)

bool fail_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_invalid_registrar_id, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::delete_domain(
                    ctx,
                    "domain.cz",
                    DefaultDeleteDomainConfigData(),
                    session_with_unauthenticated_registrar.data),
            ::Epp::EppResponseFailure,
            fail_invalid_registrar_id_exception);
}

bool fail_nonexistent_handle_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_domain_does_not_exist, supply_ctx<HasRegistrarWithSessionAndNonexistentFqdn>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::delete_domain(
                    ctx,
                    nonexistent_fqdn.fqdn,
                    DefaultDeleteDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_nonexistent_handle_exception);
}

bool fail_enum_domain_does_not_exist_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_enum_domain_does_not_exist, supply_ctx<HasRegistrarWithSessionAndNonexistentEnumFqdn>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::delete_domain(
                    ctx,
                    nonexistent_enum_fqdn.fqdn,
                    DefaultDeleteDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_enum_domain_does_not_exist_exception);
}

bool fail_wrong_registrar_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authorization_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::registrar_autor);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::unauthorized_registrar);
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_wrong_registrar, supply_ctx<HasRegistrarWithSessionAndDomainOfDifferentRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::delete_domain(
                    ctx,
                    domain_of_different_registrar.data.fqdn,
                    DefaultDeleteDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_wrong_registrar_exception);
}

bool fail_registrar_without_zone_access_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authorization_error);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_registrar_without_zone_access, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::delete_domain(
                    ctx,
                    domain.data.fqdn,
                    DefaultDeleteDomainConfigData(),
                    Session(ctx, RegistrarNotInZone(ctx).data.id).data),
            ::Epp::EppResponseFailure,
            fail_registrar_without_zone_access_exception);
}

bool fail_prohibiting_status_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_prohibiting_status, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::delete_domain(
                    ctx,
                    DomainWithStatusServerUpdateProhibited(ctx, registrar.data.handle).data.fqdn,
                    DefaultDeleteDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_prohibiting_status_exception);
}

BOOST_FIXTURE_TEST_CASE(ok, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    ::Epp::Domain::delete_domain(
        ctx,
        domain.data.fqdn,
        DefaultDeleteDomainConfigData(),
        session.data
    );

    BOOST_CHECK_EQUAL(
        ::LibFred::InfoDomainHistoryById(domain.data.id).exec(ctx).rbegin()->info_domain_data.delete_time.isnull(),
        false
    );
}

BOOST_FIXTURE_TEST_CASE(ok_states_are_upgraded, supply_ctx<HasRegistrarWithSession>)
{
    DomainWithServerTransferProhibited domain(ctx, registrar.data.handle);

    ::Epp::Domain::delete_domain(
        ctx,
        domain.data.fqdn,
        DefaultDeleteDomainConfigData(),
        session.data
    );

    // now object has the state server_transfer_prohibited request itself
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH(const ::LibFred::ObjectStateData& state, ::LibFred::GetObjectStates(domain.data.id).exec(ctx) ) {
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

BOOST_FIXTURE_TEST_CASE(ok_system_registrar_different_domain, supply_ctx<HasSystemRegistrarWithSessionAndDifferentRegistrar>)
{
    Domain domain(ctx, different_registrar.data.handle);

    ::Epp::Domain::delete_domain(
        ctx,
        domain.data.fqdn,
        DefaultDeleteDomainConfigData(),
        session.data
    );

    const auto info_domain_data = ::LibFred::InfoDomainHistoryById(domain.data.id).exec(ctx).rbegin()->info_domain_data;
    BOOST_CHECK_EQUAL(
        info_domain_data.delete_time.isnull(),
        false
    );

    try
    {
        const auto poll_msg = ::Epp::Poll::poll_request(ctx, different_registrar.data.id);
        const auto event = boost::get<::Epp::Poll::MessageEvent>(poll_msg.message);
        const auto delete_event = boost::get<::Epp::Poll::MessageEvent::Data<::Epp::Poll::MessageEvent::DeleteDomain>>(event.message);
        BOOST_CHECK_EQUAL(delete_event.handle, domain.data.fqdn);
        BOOST_CHECK_EQUAL(delete_event.date, info_domain_data.delete_time.get_value().date());
    }
    catch (...)
    {
        BOOST_FAIL("Unable to get delete domain poll message");
    }
}

BOOST_FIXTURE_TEST_CASE(ok_system_registrar_domain_lock_delete, supply_ctx<HasSystemRegistrarWithSessionAndDifferentRegistrar>)
{
    DomainWithStatusServerDeleteProhibited domain(ctx, different_registrar.data.handle);

    ::Epp::Domain::delete_domain(
        ctx,
        domain.data.fqdn,
        DefaultDeleteDomainConfigData(),
        session.data
    );

    BOOST_CHECK_EQUAL(
        ::LibFred::InfoDomainHistoryById(domain.data.id).exec(ctx).rbegin()->info_domain_data.delete_time.isnull(),
        false
    );
}

BOOST_FIXTURE_TEST_CASE(ok_system_registrar_domain_lock_update, supply_ctx<HasSystemRegistrarWithSessionAndDifferentRegistrar>)
{
    DomainWithStatusServerUpdateProhibited domain(ctx, different_registrar.data.handle);

    ::Epp::Domain::delete_domain(
        ctx,
        domain.data.fqdn,
        DefaultDeleteDomainConfigData(),
        session.data
    );

    BOOST_CHECK_EQUAL(
        ::LibFred::InfoDomainHistoryById(domain.data.id).exec(ctx).rbegin()->info_domain_data.delete_time.isnull(),
        false
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
