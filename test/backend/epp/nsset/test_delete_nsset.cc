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
#include "test/backend/epp/domain/fixture.hh"
#include "test/backend/epp/nsset/fixture.hh"
#include "test/backend/epp/util.hh"

#include "src/backend/epp/nsset/delete_nsset.hh"
#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Nsset)
BOOST_AUTO_TEST_SUITE(DeleteNsset)

bool delete_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_invalid_registrar_id, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Nsset::delete_nsset(
                    ctx,
                    ValidHandle().handle,
                    DefaultDeleteNssetConfigData(),
                    session_with_unauthenticated_registrar.data),
            ::Epp::EppResponseFailure,
            delete_invalid_registrar_id_exception);
}

bool delete_fail_nonexistent_handle_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_nonexistent_handle, supply_ctx<HasRegistrarWithSession>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Nsset::delete_nsset(
                    ctx,
                    NonexistentHandle().handle,
                    DefaultDeleteNssetConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            delete_fail_nonexistent_handle_exception);
}

bool delete_fail_wrong_registrar_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authorization_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), ::Epp::Param::registrar_autor);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), ::Epp::Reason::unauthorized_registrar);
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_wrong_registrar, supply_ctx<HasRegistrarWithSessionAndNssetOfDifferentRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Nsset::delete_nsset(
                    ctx,
                    nsset_of_different_registrar.data.handle,
                    DefaultDeleteNssetConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            delete_fail_wrong_registrar_exception);
}

bool delete_fail_prohibiting_status1_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_prohibiting_status1, supply_ctx<HasRegistrarWithSession>)
{
    NssetWithStatusServerUpdateProhibited nsset_with_status_server_update_prohibited(ctx, registrar.data.handle);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Nsset::delete_nsset(
                    ctx,
                    nsset_with_status_server_update_prohibited.data.handle,
                    DefaultDeleteNssetConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            delete_fail_prohibiting_status1_exception);
}

bool delete_fail_prohibiting_status2_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_prohibiting_status2, supply_ctx<HasRegistrarWithSession>)
{
    NssetWithStatusDeleteCandidate nsset_with_status_delete_candidate(ctx, registrar.data.handle);

    BOOST_CHECK_EXCEPTION(
            ::Epp::Nsset::delete_nsset(
                    ctx,
                    nsset_with_status_delete_candidate.data.handle,
                    DefaultDeleteNssetConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            delete_fail_prohibiting_status2_exception);
}

bool delete_fail_linked_domain_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_association_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_linked_domain, supply_ctx<HasRegistrarWithSession>)
{
    Domain::FullDomain domain(ctx, registrar.data.handle);
    BOOST_REQUIRE(!domain.data.nsset.isnull());

    BOOST_CHECK_EXCEPTION(
            ::Epp::Nsset::delete_nsset(
                    ctx,
                    domain.data.nsset.get_value().handle,
                    DefaultDeleteNssetConfigData(),
                    session.data),
            ::Epp::EppResponseFailure,
            delete_fail_linked_domain_exception);
}

BOOST_FIXTURE_TEST_CASE(delete_ok, supply_ctx<HasRegistrarWithSessionAndNsset>)
{
    ::Epp::Nsset::delete_nsset(
        ctx,
        nsset.data.handle,
        DefaultDeleteNssetConfigData(),
        session.data
    );

    BOOST_CHECK_EQUAL(
        ::LibFred::InfoNssetHistoryById(nsset.data.id).exec(ctx).rbegin()->info_nsset_data.delete_time.isnull(),
        false
    );
}

BOOST_FIXTURE_TEST_CASE(delete_ok_states_are_upgraded, supply_ctx<HasRegistrarWithSession>)
{
    NssetWithStatusRequestServerTransferProhibited nsset_with_status_request_server_transfer_prohibited(ctx, registrar.data.handle);

    ::Epp::Nsset::delete_nsset(
            ctx,
            nsset_with_status_request_server_transfer_prohibited.data.handle,
            DefaultDeleteNssetConfigData(),
            session.data);

    /* now object has the state server_transfer_prohibited request itself */
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH (const ::LibFred::ObjectStateData& state, ::LibFred::GetObjectStates(nsset_with_status_request_server_transfer_prohibited.data.id).exec(ctx))
            {
                object_states_after.push_back(state.state_name);
            }
        }

        BOOST_CHECK(
                std::find(object_states_after.begin(), object_states_after.end(), nsset_with_status_request_server_transfer_prohibited.status) !=
                object_states_after.end());
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
