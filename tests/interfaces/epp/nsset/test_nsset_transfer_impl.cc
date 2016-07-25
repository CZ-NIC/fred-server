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
#include "tests/interfaces/epp/nsset/fixture.h"

#include "src/epp/nsset/nsset_transfer_impl.h"

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(NssetTransferImpl)

BOOST_FIXTURE_TEST_CASE(transfer_fail_auth_error_srvr_closing_connection, has_nsset)
{
    BOOST_CHECK_THROW(
        Epp::nsset_transfer_impl(
            ctx,
            nsset.handle,
            nsset.authinfopw,
            0,
            42
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_nonexistent_handle, has_registrar)
{
    BOOST_CHECK_THROW(
        Epp::nsset_transfer_impl(
            ctx,
            Test::get_nonexistent_object_handle(ctx),
            "abc-it-doesnt-matter-operation-should-fail-even-sooner",
            registrar.id,
            42
        ),
        Epp::NonexistentHandle
    );
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_not_eligible_for_transfer, has_nsset)
{
    BOOST_CHECK_THROW(
        Epp::nsset_transfer_impl(
            ctx,
            nsset.handle,
            nsset.authinfopw,
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

struct has_nsset_with_server_transfer_prohibited_and_another_registrar : has_nsset_with_server_transfer_prohibited, has_another_registrar { };

BOOST_FIXTURE_TEST_CASE(transfer_fail_prohibiting_status1, has_nsset_with_server_transfer_prohibited_and_another_registrar)
{
    BOOST_CHECK_THROW(
        Epp::nsset_transfer_impl(
            ctx,
            nsset.handle,
            nsset.authinfopw,
            another_registrar.id,
            42
        ),
        Epp::ObjectStatusProhibitsOperation
    );
}

struct has_nsset_with_server_delete_prohibited_and_another_registrar : has_nsset_with_delete_candidate, has_another_registrar { };

BOOST_FIXTURE_TEST_CASE(transfer_fail_prohibiting_status2, has_nsset_with_server_delete_prohibited_and_another_registrar)
{
    BOOST_CHECK_THROW(
        Epp::nsset_transfer_impl(
            ctx,
            nsset.handle,
            nsset.authinfopw,
            another_registrar.id,
            42
        ),
        Epp::ObjectStatusProhibitsOperation
    );
}

struct has_nsset_and_another_registrar : has_nsset, has_another_registrar { };

BOOST_FIXTURE_TEST_CASE(transfer_fail_authinfo_error, has_nsset_and_another_registrar)
{
    BOOST_CHECK_THROW(
        Epp::nsset_transfer_impl(
            ctx,
            nsset.handle,
            "thisisdifferent" + nsset.authinfopw,
            another_registrar.id,
            42
        ),
        Epp::AuthorizationInformationError
    );
}

struct has_nsset_with_server_update_prohibited_request_and_another_registrar : has_another_registrar, has_nsset_with_server_update_prohibited_request { };

BOOST_FIXTURE_TEST_CASE(transfer_ok_state_requests_updated, has_nsset_with_server_update_prohibited_request_and_another_registrar)
{
    Epp::nsset_transfer_impl(
        ctx,
        nsset.handle,
        nsset.authinfopw,
        another_registrar.id,
        42
    );

    /* now object has the state server_update_prohibited itself */
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(nsset.id).exec(ctx) ) {
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

BOOST_FIXTURE_TEST_CASE(transfer_ok_full_data, has_nsset_and_another_registrar)
{
    const Fred::InfoNssetData nsset_data_before = Fred::InfoNssetByHandle(nsset.handle).exec(ctx).info_nsset_data;

    Epp::nsset_transfer_impl(
        ctx,
        nsset.handle,
        nsset.authinfopw,
        another_registrar.id,
        42
    );

    const Fred::InfoNssetData nsset_data_after = Fred::InfoNssetByHandle(nsset.handle).exec(ctx).info_nsset_data;

    const Fred::InfoNssetDiff nsset_data_change = diff_nsset_data(nsset_data_before, nsset_data_after);
    const std::set<std::string> change_fields_etalon = boost::assign::list_of
        ("sponsoring_registrar_handle")
        ("transfer_time")
        ("historyid")
        ("authinfopw");

    BOOST_CHECK_EQUAL(nsset_data_change.changed_fields(), change_fields_etalon);

    BOOST_CHECK_EQUAL(nsset_data_after.sponsoring_registrar_handle, another_registrar.handle);

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

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
