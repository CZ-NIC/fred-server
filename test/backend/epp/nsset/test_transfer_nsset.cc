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
/**
 *  @file
 */

#include "test/backend/epp/fixture.hh"
#include "test/backend/epp/util.hh"
#include "test/backend/epp/nsset/fixture.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/nsset/transfer_nsset.hh"

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Nsset)
BOOST_AUTO_TEST_SUITE(TransferNsset)

bool transfer_fail_auth_error_srvr_closing_connection_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_auth_error_srvr_closing_connection, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Nsset::transfer_nsset(
                    ctx,
                    ValidHandle().handle,
                    "authinfopw",
                    DefaultTransferNssetConfigData(),
                    session_with_unauthenticated_registrar.data),
            ::Epp::EppResponseFailure,
            transfer_fail_auth_error_srvr_closing_connection_exception);
}

bool transfer_fail_nonexistent_handle_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_nonexistent_handle, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Nsset::transfer_nsset(
                    ctx,
                    Test::get_nonexistent_object_handle(ctx),
                    "authinfopw",
                    DefaultTransferNssetConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            transfer_fail_nonexistent_handle_exception);
}

bool transfer_fail_not_eligible_for_transfer_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_is_not_eligible_for_transfer);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_not_eligible_for_transfer, supply_ctx<HasRegistrarWithSessionAndNsset>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Nsset::transfer_nsset(
                    ctx,
                    nsset.data.handle,
                    nsset.data.authinfopw,
                    DefaultTransferNssetConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            transfer_fail_not_eligible_for_transfer_exception);
}

bool transfer_fail_prohibiting_status1_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_prohibiting_status1, supply_ctx<HasRegistrarWithSessionAndDifferentRegistrar>)
{
    NssetWithStatusServerTransferProhibited nsset_of_different_registrar_and_with_status_server_transfer_prohibited(ctx, different_registrar.data.handle);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Nsset::transfer_nsset(
                    ctx,
                    nsset_of_different_registrar_and_with_status_server_transfer_prohibited.data.handle,
                    nsset_of_different_registrar_and_with_status_server_transfer_prohibited.data.authinfopw,
                    DefaultTransferNssetConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            transfer_fail_prohibiting_status1_exception);
}

bool transfer_fail_prohibiting_status2_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_prohibiting_status2, supply_ctx<HasRegistrarWithSessionAndDifferentRegistrar>)
{
    NssetWithStatusDeleteCandidate nsset_of_different_registrar_and_with_status_delete_candidate(ctx, different_registrar.data.handle);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Nsset::transfer_nsset(
                    ctx,
                    nsset_of_different_registrar_and_with_status_delete_candidate.data.handle,
                    nsset_of_different_registrar_and_with_status_delete_candidate.data.authinfopw,
                    DefaultTransferNssetConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            transfer_fail_prohibiting_status2_exception);
}

bool transfer_fail_authinfopw_error_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::invalid_authorization_information);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_authinfopw_error, supply_ctx<HasRegistrarWithSessionAndNssetOfDifferentRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Nsset::transfer_nsset(
                    ctx,
                    nsset_of_different_registrar.data.handle,
                    "thisisdifferent" + nsset_of_different_registrar.data.authinfopw,
                    DefaultTransferNssetConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            transfer_fail_authinfopw_error_exception);
}

BOOST_FIXTURE_TEST_CASE(transfer_ok_state_requests_updated, supply_ctx<HasRegistrarWithSessionAndDifferentRegistrar>)
{
    NssetWithStatusRequestServerUpdateProhibited nsset_of_different_registrar_and_with_status_request_server_update_prohibited(ctx, different_registrar.data.handle);

    ::Epp::Nsset::transfer_nsset(
            ctx,
            nsset_of_different_registrar_and_with_status_request_server_update_prohibited.data.handle,
            nsset_of_different_registrar_and_with_status_request_server_update_prohibited.data.authinfopw,
            DefaultTransferNssetConfigData(),
            session.data);

    /* now object has the state server_update_prohibited itself */
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH(const ::LibFred::ObjectStateData& state, ::LibFred::GetObjectStates(nsset_of_different_registrar_and_with_status_request_server_update_prohibited.data.id).exec(ctx) ) {
                object_states_after.push_back(state.state_name);
            }
        }

        BOOST_CHECK(
            std::find(object_states_after.begin(), object_states_after.end(), nsset_of_different_registrar_and_with_status_request_server_update_prohibited.status)
            !=
            object_states_after.end()
        );
    }
}

BOOST_FIXTURE_TEST_CASE(transfer_ok_full_data, supply_ctx<HasRegistrarWithSessionAndDifferentRegistrar>)
{
    FullNsset nsset_of_different_registrar(ctx, different_registrar.data.handle);
    const ::LibFred::InfoNssetData nsset_data_before = ::LibFred::InfoNssetByHandle(nsset_of_different_registrar.data.handle).exec(ctx).info_nsset_data;

    ::Epp::Nsset::transfer_nsset(
        ctx,
        nsset_of_different_registrar.data.handle,
        nsset_of_different_registrar.data.authinfopw,
        DefaultTransferNssetConfigData(),
        session.data
    );

    const ::LibFred::InfoNssetData nsset_data_after = ::LibFred::InfoNssetByHandle(nsset_of_different_registrar.data.handle).exec(ctx).info_nsset_data;

    const ::LibFred::InfoNssetDiff nsset_data_change = diff_nsset_data(nsset_data_before, nsset_data_after);
    const std::set<std::string> change_fields_etalon =
            {
                "sponsoring_registrar_handle",
                "transfer_time",
                "historyid",
                "history_uuid",
                "authinfopw"
            };

    BOOST_CHECK(nsset_data_change.changed_fields() == change_fields_etalon);

    BOOST_CHECK_EQUAL(nsset_data_after.sponsoring_registrar_handle, registrar.data.handle);

    BOOST_CHECK_EQUAL(
        nsset_data_after.transfer_time,
        boost::posix_time::time_from_string(
            static_cast<std::string>(
                ctx.get_conn().exec(
                    "SELECT now()::timestamp without time zone AT TIME ZONE 'UTC' AT TIME ZONE 'Europe/Prague' "
                )[0][0]
            )
        )
    );

    BOOST_CHECK(nsset_data_before.historyid != nsset_data_after.historyid);

    BOOST_CHECK(nsset_data_before.authinfopw != nsset_data_after.authinfopw);
}

BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Nsset/TransferNsset
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Nsset
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp
BOOST_AUTO_TEST_SUITE_END()//Backend

}//namespace Test
