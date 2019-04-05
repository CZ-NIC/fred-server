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

#include "src/backend/epp/domain/transfer_domain.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/param.hh"
#include "src/backend/epp/reason.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include <set>
#include <string>
#include <vector>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(TransferDomain)

bool fail_auth_error_srvr_closing_connection_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_auth_error_srvr_closing_connection, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::transfer_domain(
                    ctx,
                    "domain.cz",
                    "authinfopw",
                    DefaultTransferDomainConfigData(),
                    session_with_unauthenticated_registrar.data),
            ::Epp::EppResponseFailure,
            fail_auth_error_srvr_closing_connection_exception);
}

bool fail_nonexistent_handle_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_domain_does_not_exist, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::transfer_domain(
                    ctx,
                    get_nonexistent_object_handle(ctx),
                    "authinfopw",
                    DefaultTransferDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_nonexistent_handle_exception);
}

bool fail_enum_domain_does_not_exist_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_enum_domain_does_not_exist, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::transfer_domain(
                    ctx,
                    NonexistentEnumFqdn().fqdn,
                    "authinfopw",
                    DefaultTransferDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_enum_domain_does_not_exist_exception);
}

bool fail_not_eligible_for_transfer_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_is_not_eligible_for_transfer);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_not_eligible_for_transfer, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::transfer_domain(
                    ctx,
                    domain.data.fqdn,
                    domain.data.authinfopw,
                    DefaultTransferDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_not_eligible_for_transfer_exception);
}

bool fail_prohibiting_status1_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_prohibiting_status1, supply_ctx<HasRegistrarWithSessionAndDifferentRegistrar>)
{
    const DomainWithServerTransferProhibited domain_with_server_transfer_prohibited(ctx, different_registrar.data.handle);

    ::LibFred::PerformObjectStateRequest(domain_with_server_transfer_prohibited.data.id).exec(ctx);
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::transfer_domain(
                    ctx,
                    domain_with_server_transfer_prohibited.data.fqdn,
                    domain_with_server_transfer_prohibited.data.authinfopw,
                    DefaultTransferDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_prohibiting_status1_exception);
}

bool fail_authz_info_error_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::invalid_authorization_information);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_authz_info_error, supply_ctx<HasRegistrarWithSessionAndDomainOfDifferentRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::transfer_domain(
                    ctx,
                    domain_of_different_registrar.data.fqdn,
                    "thisisdifferent" + domain_of_different_registrar.data.authinfopw,
                    DefaultTransferDomainConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            fail_authz_info_error_exception);
}
bool fail_registrar_without_zone_access_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authorization_error);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(fail_registrar_without_zone_access, supply_ctx<HasRegistrarWithSessionAndDomain>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Domain::transfer_domain(
                    ctx,
                    domain.data.fqdn,
                    domain.data.authinfopw,
                    DefaultTransferDomainConfigData(),
                    Session(ctx, RegistrarNotInZone(ctx).data.id).data),
            ::Epp::EppResponseFailure,
            fail_registrar_without_zone_access_exception);
}

BOOST_FIXTURE_TEST_CASE(ok_state_requests_updated, supply_ctx<HasRegistrarWithSessionAndDifferentRegistrar>)
{
    const DomainWithStatusRequestServerUpdateProhibited domain_of_different_registrar_and_with_server_update_prohibited_request(ctx, different_registrar.data.handle);

    ::LibFred::PerformObjectStateRequest(domain_of_different_registrar_and_with_server_update_prohibited_request.data.id).exec(ctx);
    ::Epp::Domain::transfer_domain(
        ctx,
        domain_of_different_registrar_and_with_server_update_prohibited_request.data.fqdn,
        domain_of_different_registrar_and_with_server_update_prohibited_request.data.authinfopw,
        DefaultTransferDomainConfigData(),
        session.data
    );

    // now object has the state server_update_prohibited itself
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH(const ::LibFred::ObjectStateData& state, ::LibFred::GetObjectStates(domain_of_different_registrar_and_with_server_update_prohibited_request.data.id).exec(ctx) ) {
                object_states_after.push_back(state.state_name);
            }
        }

        BOOST_CHECK(
            std::find(object_states_after.begin(), object_states_after.end(), domain_of_different_registrar_and_with_server_update_prohibited_request.status)
            !=
            object_states_after.end()
        );
    }
}

BOOST_FIXTURE_TEST_CASE(ok_transfer_prohibited_but_system_registrar, supply_ctx<HasSystemRegistrarWithSessionAndDifferentRegistrar>)
{
    const DomainWithStatusRequestServerUpdateProhibited domain_of_different_registrar_and_with_server_update_prohibited_request(ctx, Registrar(ctx).data.handle);
    const ::LibFred::InfoDomainData domain_data_before = ::LibFred::InfoDomainByFqdn(domain_of_different_registrar_and_with_server_update_prohibited_request.data.fqdn).exec(ctx).info_domain_data;

    ::Epp::Domain::transfer_domain(
        ctx,
        domain_of_different_registrar_and_with_server_update_prohibited_request.data.fqdn,
        domain_of_different_registrar_and_with_server_update_prohibited_request.data.authinfopw,
        DefaultTransferDomainConfigData(),
        session.data
    );

    const ::LibFred::InfoDomainData domain_data_after = ::LibFred::InfoDomainByFqdn(domain_data_before.fqdn).exec(ctx).info_domain_data;

    const ::LibFred::InfoDomainDiff domain_data_change = diff_domain_data(domain_data_before, domain_data_after);
    const std::set<std::string> change_fields_etalon =
            {
                "sponsoring_registrar_handle",
                "transfer_time",
                "historyid",
                "history_uuid",
                "authinfopw"
            };

    BOOST_CHECK(domain_data_change.changed_fields() == change_fields_etalon);

    BOOST_CHECK_EQUAL(domain_data_after.sponsoring_registrar_handle, system_registrar.data.handle);

    BOOST_CHECK_EQUAL(
        domain_data_after.transfer_time,
        boost::posix_time::time_from_string(
            static_cast<std::string>(
                ctx.get_conn().exec(
                    "SELECT NOW()::TIMESTAMP WITHOUT TIME ZONE AT TIME ZONE 'UTC' AT TIME ZONE 'Europe/Prague' "
                )[0][0]
            )
        )
    );

    BOOST_CHECK(domain_data_before.historyid != domain_data_after.historyid);

    BOOST_CHECK(domain_data_before.authinfopw != domain_data_after.authinfopw);
}

BOOST_FIXTURE_TEST_CASE(ok, supply_ctx<HasRegistrarWithSessionAndDomainOfDifferentRegistrar>)
{
    const ::LibFred::InfoDomainData domain_data_before = ::LibFred::InfoDomainByFqdn(domain_of_different_registrar.data.fqdn).exec(ctx).info_domain_data;

    ::Epp::Domain::transfer_domain(
        ctx,
        domain_of_different_registrar.data.fqdn,
        domain_of_different_registrar.data.authinfopw,
        DefaultTransferDomainConfigData(),
        session.data
    );

    const ::LibFred::InfoDomainData domain_data_after = ::LibFred::InfoDomainByFqdn(domain_data_before.fqdn).exec(ctx).info_domain_data;

    const ::LibFred::InfoDomainDiff domain_data_change = diff_domain_data(domain_data_before, domain_data_after);
    const std::set<std::string> change_fields_etalon =
            {
                "sponsoring_registrar_handle",
                "transfer_time",
                "historyid",
                "history_uuid",
                "authinfopw"
            };

    BOOST_CHECK(domain_data_change.changed_fields() == change_fields_etalon);

    BOOST_CHECK_EQUAL(domain_data_after.sponsoring_registrar_handle, registrar.data.handle);

    BOOST_CHECK_EQUAL(
        domain_data_after.transfer_time,
        boost::posix_time::time_from_string(
            static_cast<std::string>(
                ctx.get_conn().exec(
                    "SELECT NOW()::TIMESTAMP WITHOUT TIME ZONE AT TIME ZONE 'UTC' AT TIME ZONE 'Europe/Prague' "
                )[0][0]
            )
        )
    );

    BOOST_CHECK_NE(domain_data_before.historyid, domain_data_after.historyid);
    BOOST_CHECK_NE(domain_data_before.authinfopw, domain_data_after.authinfopw);
}

BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Domain/TransferDomain
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Domain
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp
BOOST_AUTO_TEST_SUITE_END()//Backend

}//namespace Test
