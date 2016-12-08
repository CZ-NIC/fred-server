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

/**
 *  @file
 */

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>

#include "tests/interfaces/epp/util.h"
#include "tests/interfaces/epp/contact/fixture.h"

#include "src/epp/contact/transfer_contact.h"

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(ContactTransferImpl)

BOOST_FIXTURE_TEST_CASE(transfer_fail_auth_error_srvr_closing_connection, has_contact)
{
    BOOST_CHECK_THROW(
        Epp::Contact::transfer_contact(
            ctx,
            contact.handle,
            contact.authinfopw,
            0,
            42
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_nonexistent_handle, has_registrar)
{
    BOOST_CHECK_THROW(
        Epp::Contact::transfer_contact(
            ctx,
            Test::get_nonexistent_object_handle(ctx),
            "abc-it-doesnt-matter-operation-should-fail-even-sooner",
            registrar.id,
            42
        ),
        Epp::NonexistentHandle
    );
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_not_eligible_for_transfer, has_contact)
{
    BOOST_CHECK_THROW(
        Epp::Contact::transfer_contact(
            ctx,
            contact.handle,
            contact.authinfopw,
            registrar.id,
            42
        ),
        Epp::ObjectNotEligibleForTransfer
    );
}

struct has_another_registrar : virtual Test::autocommitting_context {
    Fred::InfoRegistrarData another_registrar;

    has_another_registrar() {
        const std::string reg_handle = "ANOTHERREGISTRAR";
        Fred::CreateRegistrar(reg_handle).exec(ctx);
        another_registrar = Fred::InfoRegistrarByHandle(reg_handle).exec(ctx).info_registrar_data;
    }
};

struct has_contact_with_server_transfer_prohibited_and_another_registrar : has_contact_with_server_transfer_prohibited, has_another_registrar { };

BOOST_FIXTURE_TEST_CASE(transfer_fail_prohibiting_status1, has_contact_with_server_transfer_prohibited_and_another_registrar)
{
    BOOST_CHECK_THROW(
        Epp::Contact::transfer_contact(
            ctx,
            contact.handle,
            contact.authinfopw,
            another_registrar.id,
            42
        ),
        Epp::ObjectStatusProhibitsOperation
    );
}

struct has_contact_with_server_delete_prohibited_and_another_registrar : has_contact_with_delete_candidate, has_another_registrar { };

BOOST_FIXTURE_TEST_CASE(transfer_fail_prohibiting_status2, has_contact_with_server_delete_prohibited_and_another_registrar)
{
    BOOST_CHECK_THROW(
        Epp::Contact::transfer_contact(
            ctx,
            contact.handle,
            contact.authinfopw,
            another_registrar.id,
            42
        ),
        Epp::ObjectStatusProhibitsOperation
    );
}

struct has_contact_and_another_registrar : has_contact, has_another_registrar { };

BOOST_FIXTURE_TEST_CASE(transfer_fail_autor_error, has_contact_and_another_registrar)
{
    BOOST_CHECK_THROW(
        Epp::Contact::transfer_contact(
            ctx,
            contact.handle,
            "thisisdifferent" + contact.authinfopw,
            another_registrar.id,
            42
        ),
        Epp::AuthorizationError
    );
}

struct has_contact_with_server_update_prohibited_request_and_another_registrar : has_another_registrar, has_contact_with_server_update_prohibited_request { };

BOOST_FIXTURE_TEST_CASE(transfer_ok_state_requests_updated, has_contact_with_server_update_prohibited_request_and_another_registrar)
{
    Epp::Contact::transfer_contact(
        ctx,
        contact.handle,
        contact.authinfopw,
        another_registrar.id,
        42
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

BOOST_FIXTURE_TEST_CASE(transfer_ok_full_data, has_contact_and_another_registrar)
{
    const Fred::InfoContactData contact_data_before = Fred::InfoContactByHandle(contact.handle).exec(ctx).info_contact_data;

    Epp::Contact::transfer_contact(
        ctx,
        contact.handle,
        contact.authinfopw,
        another_registrar.id,
        42
    );

    const Fred::InfoContactData contact_data_after = Fred::InfoContactByHandle(contact.handle).exec(ctx).info_contact_data;

    const Fred::InfoContactDiff contact_data_change = diff_contact_data(contact_data_before, contact_data_after);
    const std::set<std::string> change_fields_etalon = boost::assign::list_of
        ("sponsoring_registrar_handle")
        ("transfer_time")
        ("historyid")
        ("authinfopw");

    BOOST_CHECK(contact_data_change.changed_fields() == change_fields_etalon);

    BOOST_CHECK_EQUAL(contact_data_after.sponsoring_registrar_handle, another_registrar.handle);

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
