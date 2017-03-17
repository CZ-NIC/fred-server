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

#include "src/epp/contact/transfer_contact.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(TransferContact)

bool transfer_fail_auth_error_srvr_closing_connection_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_auth_error_srvr_closing_connection, HasInvalidSessionRegistrar)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Contact::transfer_contact(
            ctx,
            "contact_handle",
            "authinfopw",
            config.transfer_contact_config_data,
            session.data
        ),
        Epp::EppResponseFailure,
        transfer_fail_auth_error_srvr_closing_connection_exception
    );
}

bool transfer_fail_nonexistent_handle_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_nonexistent_handle, HasRegistrar)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Contact::transfer_contact(
            ctx,
            Test::get_nonexistent_object_handle(ctx),
            "abc-it-doesnt-matter-operation-should-fail-even-sooner",
            config.transfer_contact_config_data,
            session.data
        ),
        Epp::EppResponseFailure,
        transfer_fail_nonexistent_handle_exception
    );
}

bool transfer_fail_not_eligible_for_transfer_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_is_not_eligible_for_transfer);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_not_eligible_for_transfer, HasContact)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Contact::transfer_contact(
            ctx,
            contact.handle,
            contact.authinfopw,
            config.transfer_contact_config_data,
            session.data
        ),
        Epp::EppResponseFailure,
        transfer_fail_not_eligible_for_transfer_exception
    );
}

bool transfer_fail_prohibiting_status1_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_prohibiting_status1, HasContactWithServerTransferProhibited)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Contact::transfer_contact(
            ctx,
            contact.handle,
            contact.authinfopw,
            config.transfer_contact_config_data,
            Test::Session(ctx, Test::Registrar(ctx).data.id).data
        ),
        Epp::EppResponseFailure,
        transfer_fail_prohibiting_status1_exception
    );
}

bool transfer_fail_prohibiting_status2_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_prohibiting_status2, HasContactWithDeleteCandidate)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Contact::transfer_contact(
            ctx,
            contact.handle,
            contact.authinfopw,
            config.transfer_contact_config_data,
            Test::Session(ctx, Test::Registrar(ctx).data.id).data
        ),
        Epp::EppResponseFailure,
        transfer_fail_prohibiting_status2_exception
    );
}

bool transfer_fail_autor_error_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::invalid_authorization_information);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_autor_error, HasContact)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Contact::transfer_contact(
            ctx,
            contact.handle,
            "thisisdifferent" + contact.authinfopw,
            config.transfer_contact_config_data,
            Test::Session(ctx, Test::Registrar(ctx).data.id).data
        ),
        Epp::EppResponseFailure,
        transfer_fail_autor_error_exception
    );
}

BOOST_FIXTURE_TEST_CASE(transfer_ok_state_requests_updated, HasContactWithServerUpdateProhibitedRequest)
{
    Epp::Contact::transfer_contact(
        ctx,
        contact.handle,
        contact.authinfopw,
        config.transfer_contact_config_data,
        Test::Session(ctx, Test::Registrar(ctx).data.id).data
    );

    /* now object has the state server_update_prohibited itself */
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(contact.id).exec(ctx) ) {
                object_states_after.push_back(state.state_name);
            }
        }

        BOOST_CHECK(
            std::find( object_states_after.begin(), object_states_after.end(), status )
            !=
            object_states_after.end()
        );
    }
}

BOOST_FIXTURE_TEST_CASE(transfer_ok_full_data, HasContact)
{
    const Fred::InfoContactData contact_data_before = Fred::InfoContactByHandle(contact.handle).exec(ctx).info_contact_data;

    Test::Registrar another_registrar(ctx);

    Epp::Contact::transfer_contact(
        ctx,
        contact.handle,
        contact.authinfopw,
        config.transfer_contact_config_data,
        Test::Session(ctx, another_registrar.data.id).data
    );

    const Fred::InfoContactData contact_data_after = Fred::InfoContactByHandle(contact.handle).exec(ctx).info_contact_data;

    const Fred::InfoContactDiff contact_data_change = diff_contact_data(contact_data_before, contact_data_after);
    const std::set<std::string> change_fields_etalon = boost::assign::list_of
        ("sponsoring_registrar_handle")
        ("transfer_time")
        ("historyid")
        ("authinfopw");

    BOOST_CHECK(contact_data_change.changed_fields() == change_fields_etalon);

    BOOST_CHECK_EQUAL(contact_data_after.sponsoring_registrar_handle, another_registrar.data.handle);

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
