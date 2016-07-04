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

#include "src/epp/nsset/nsset_delete_impl.h"

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(NssetDeleteImpl)

BOOST_FIXTURE_TEST_CASE(delete_invalid_registrar_id, has_nsset)
{
    BOOST_CHECK_THROW(
        Epp::nsset_delete_impl(
            ctx,
            nsset.handle,
            0
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(delete_fail_nonexistent_handle, has_nsset)
{
    BOOST_CHECK_THROW(
        Epp::nsset_delete_impl(
            ctx,
            "SOMEobscureString",
            registrar.id
        ),
        Epp::NonexistentHandle
    );
}

BOOST_FIXTURE_TEST_CASE(delete_fail_wrong_registrar, has_nsset_and_a_different_registrar)
{
    BOOST_CHECK_THROW(
        Epp::nsset_delete_impl(
            ctx,
            nsset.handle,
            the_different_registrar.id
        ),
        Epp::AutorError
    );
}

BOOST_FIXTURE_TEST_CASE(delete_fail_prohibiting_status1, has_nsset_with_server_update_prohibited)
{
    BOOST_CHECK_THROW(
        Epp::nsset_delete_impl(
            ctx,
            nsset.handle,
            registrar.id
        ),
        Epp::ObjectStatusProhibitingOperation
    );
}

struct has_nsset_with_server_delete_prohibited : has_nsset_with_status {
    has_nsset_with_server_delete_prohibited()
    :   has_nsset_with_status("serverDeleteProhibited")
    { }
};

BOOST_FIXTURE_TEST_CASE(delete_fail_prohibiting_status2, has_nsset_with_delete_candidate)
{
    BOOST_CHECK_THROW(
        Epp::nsset_delete_impl(
            ctx,
            nsset.handle,
            registrar.id
        ),
        Epp::ObjectStatusProhibitingOperation
    );
}
struct has_nsset_linked_to_domain : has_nsset_with_all_data_set {
    has_nsset_linked_to_domain() {
        Fred::CreateDomain("domain.cz", registrar.handle, nsset.tech_contacts.at(0).handle).set_nsset(nsset.handle).exec(ctx);
    }
};

BOOST_FIXTURE_TEST_CASE(delete_fail_linked_domain, has_nsset_linked_to_domain)
{
    BOOST_CHECK_THROW(
        Epp::nsset_delete_impl(
            ctx,
            nsset.handle,
            registrar.id
        ),
        Epp::ObjectAssotiationProhibitsOperation
    );
}

BOOST_FIXTURE_TEST_CASE(delete_ok, has_nsset)
{
    Epp::nsset_delete_impl(
        ctx,
        nsset.handle,
        registrar.id
    );

    BOOST_CHECK_EQUAL(
        Fred::InfoNssetHistoryById(nsset.id).exec(ctx).rbegin()->info_nsset_data.delete_time.isnull(),
        false
    );
}

BOOST_FIXTURE_TEST_CASE(delete_ok_states_are_upgraded, has_nsset_with_server_transfer_prohibited_request)
{
    Epp::nsset_delete_impl(
        ctx,
        nsset.handle,
        registrar.id
    );

    /* now object has the state server_transfer_prohibited request itself */
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

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
