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

#include "tests/interfaces/epp/nsset/fixture.h"
#include "tests/interfaces/epp/util.h"

#include "src/epp/nsset/delete_nsset.h"
#include "src/epp/impl/epp_response_failure.h"
#include "src/epp/impl/epp_result_code.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Nsset)
BOOST_AUTO_TEST_SUITE(DeleteNsset)

bool delete_invalid_registrar_id_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_invalid_registrar_id, has_nsset)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::delete_nsset(
            ctx,
            nsset.handle,
            0
        ),
        Epp::EppResponseFailure,
        delete_invalid_registrar_id_exception
    );
}

bool delete_fail_nonexistent_handle_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_nonexistent_handle, has_nsset)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::delete_nsset(
            ctx,
            "SOMEobscureString",
            registrar.id
        ),
        Epp::EppResponseFailure,
        delete_fail_nonexistent_handle_exception
    );
}

bool delete_fail_wrong_registrar_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authorization_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->size(), 1);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->param(), Epp::Param::registrar_autor);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->position(), 0);
    BOOST_CHECK_EQUAL(e.epp_result().extended_errors()->begin()->reason(), Epp::Reason::unauthorized_registrar);
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_wrong_registrar, has_nsset_and_a_different_registrar)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::delete_nsset(
            ctx,
            nsset.handle,
            the_different_registrar.id
        ),
        Epp::EppResponseFailure,
        delete_fail_wrong_registrar_exception
    );
}

bool delete_fail_prohibiting_status1_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_prohibiting_status1, has_nsset_with_server_update_prohibited)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::delete_nsset(
            ctx,
            nsset.handle,
            registrar.id
        ),
        Epp::EppResponseFailure,
        delete_fail_prohibiting_status1_exception
    );
}

struct has_nsset_with_server_delete_prohibited : has_nsset_with_status {
    has_nsset_with_server_delete_prohibited()
    :   has_nsset_with_status("serverDeleteProhibited")
    { }
};

bool delete_fail_prohibiting_status2_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_status_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_prohibiting_status2, has_nsset_with_delete_candidate)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::delete_nsset(
            ctx,
            nsset.handle,
            registrar.id
        ),
        Epp::EppResponseFailure,
        delete_fail_prohibiting_status2_exception
    );
}
struct has_nsset_linked_to_domain : has_nsset_with_all_data_set {
    has_nsset_linked_to_domain() {
        Fred::CreateDomain("domain.cz", registrar.handle, nsset.tech_contacts.at(0).handle).set_nsset(nsset.handle).exec(ctx);
    }
};

bool delete_fail_linked_domain_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_association_prohibits_operation);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(delete_fail_linked_domain, has_nsset_linked_to_domain)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::delete_nsset(
            ctx,
            nsset.handle,
            registrar.id
        ),
        Epp::EppResponseFailure,
        delete_fail_linked_domain_exception
    );
}

BOOST_FIXTURE_TEST_CASE(delete_ok, has_nsset)
{
    Epp::Nsset::delete_nsset(
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
    Epp::Nsset::delete_nsset(
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
