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
#include "test/backend/epp/contact/fixture.hh"
#include "test/backend/epp/util.hh"

#include "src/backend/epp/contact/transfer_contact.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(TransferContact)

bool transfer_fail_auth_error_srvr_closing_connection_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_auth_error_srvr_closing_connection, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::transfer_contact(
                    ctx,
                    ValidHandle().handle,
                    "authinfopw",
                    DefaultTransferContactConfigData(),
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
            ::Epp::Contact::transfer_contact(
                    ctx,
                    Test::get_nonexistent_object_handle(ctx),
                    "authinfopw",
                    DefaultTransferContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            transfer_fail_nonexistent_handle_exception);
}

bool transfer_fail_not_eligible_for_transfer_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_is_not_eligible_for_transfer);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_not_eligible_for_transfer, supply_ctx<HasRegistrarWithSessionAndContact>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::transfer_contact(
                    ctx,
                    contact.data.handle,
                    contact.data.authinfopw,
                    DefaultTransferContactConfigData(),
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
    ContactWithStatusServerTransferProhibited contact_of_different_registrar_and_with_status_server_transfer_prohibited(ctx, different_registrar.data.handle);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::transfer_contact(
                    ctx,
                    contact_of_different_registrar_and_with_status_server_transfer_prohibited.data.handle,
                    contact_of_different_registrar_and_with_status_server_transfer_prohibited.data.authinfopw,
                    DefaultTransferContactConfigData(),
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
    ContactWithStatusDeleteCandidate contact_of_different_registrar_and_with_status_delete_candidate(ctx, different_registrar.data.handle);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::transfer_contact(
                    ctx,
                    contact_of_different_registrar_and_with_status_delete_candidate.data.handle,
                    contact_of_different_registrar_and_with_status_delete_candidate.data.authinfopw,
                    DefaultTransferContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            transfer_fail_prohibiting_status2_exception);
}

bool transfer_fail_autor_error_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::invalid_authorization_information);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(transfer_ok_state_requests_updated, supply_ctx<HasRegistrarWithSessionAndDifferentRegistrar>)
{
    ContactWithStatusRequestServerUpdateProhibited contact_of_different_registrar_and_with_status_request_server_update_prohibited(ctx, different_registrar.data.handle);

    ::Epp::Contact::transfer_contact(
            ctx,
            contact_of_different_registrar_and_with_status_request_server_update_prohibited.data.handle,
            contact_of_different_registrar_and_with_status_request_server_update_prohibited.data.authinfopw,
            DefaultTransferContactConfigData(),
            session.data);

    // now object has the state server_update_prohibited itself
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH (const ::LibFred::ObjectStateData& state, ::LibFred::GetObjectStates(contact_of_different_registrar_and_with_status_request_server_update_prohibited.data.id).exec(ctx))
            {
                object_states_after.push_back(state.state_name);
            }
        }

        BOOST_CHECK(
            std::find( object_states_after.begin(), object_states_after.end(), contact_of_different_registrar_and_with_status_request_server_update_prohibited.status )
            !=
            object_states_after.end()
        );
    }
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_autor_error, supply_ctx<HasRegistrarWithSessionAndContactOfDifferentRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Contact::transfer_contact(
                    ctx,
                    contact_of_different_registrar.data.handle,
                    "thisisdifferent" + contact_of_different_registrar.data.authinfopw,
                    DefaultTransferContactConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            transfer_fail_autor_error_exception);
}

BOOST_FIXTURE_TEST_CASE(transfer_ok_full_data, supply_ctx<HasRegistrarWithSessionAndContactOfDifferentRegistrar>)
{
    const ::LibFred::InfoContactData contact_data_before = ::LibFred::InfoContactByHandle(contact_of_different_registrar.data.handle).exec(ctx).info_contact_data;

    ::Epp::Contact::transfer_contact(
            ctx,
            contact_of_different_registrar.data.handle,
            contact_of_different_registrar.data.authinfopw,
            DefaultTransferContactConfigData(),
            session.data);

    const ::LibFred::InfoContactData contact_data_after = ::LibFred::InfoContactByHandle(contact_of_different_registrar.data.handle).exec(ctx).info_contact_data;

    const ::LibFred::InfoContactDiff contact_data_change = diff_contact_data(contact_data_before, contact_data_after);
    const std::set<std::string> change_fields_etalon = boost::assign::list_of
        ("sponsoring_registrar_handle")
        ("transfer_time")
        ("historyid")
        ("authinfopw");

    BOOST_CHECK(contact_data_change.changed_fields() == change_fields_etalon);

    BOOST_CHECK_EQUAL(contact_data_after.sponsoring_registrar_handle, registrar.data.handle);

    BOOST_CHECK_EQUAL(
        contact_data_after.transfer_time,
        boost::posix_time::time_from_string(
            static_cast<std::string>(
                ctx.get_conn().exec(
                    "SELECT now()::timestamp without time zone AT TIME ZONE 'UTC' AT TIME ZONE 'Europe/Prague' "
                )[0][0]
            )
        )
    );

    BOOST_CHECK(contact_data_before.historyid != contact_data_after.historyid);

    BOOST_CHECK(contact_data_before.authinfopw != contact_data_after.authinfopw);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
