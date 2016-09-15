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
#include "tests/interfaces/epp/domain/fixture.h"

#include "src/epp/domain/domain_transfer_impl.h"

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(DomainTransferImpl)

BOOST_FIXTURE_TEST_CASE(transfer_fail_auth_error_srvr_closing_connection, HasInfoDomainData)
{
    BOOST_CHECK_THROW(
        Epp::Domain::domain_transfer_impl(
            ctx,
            info_domain_data.fqdn,
            info_domain_data.authinfopw,
            0,
            42
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_nonexistent_handle, HasInfoRegistrarData)
{
    BOOST_CHECK_THROW(
        Epp::Domain::domain_transfer_impl(
            ctx,
            Test::get_nonexistent_object_handle(ctx),
            "abc-it-doesnt-matter-operation-should-fail-even-sooner",
            info_registrar_data.id,
            42
        ),
        Epp::NonexistentHandle
    );
}

BOOST_FIXTURE_TEST_CASE(transfer_fail_not_eligible_for_transfer, HasInfoDomainData)
{
    BOOST_CHECK_THROW(
        Epp::Domain::domain_transfer_impl(
            ctx,
            info_domain_data.fqdn,
            info_domain_data.authinfopw,
            info_registrar_data.id,
            42
        ),
        Epp::ObjectNotEligibleForTransfer
    );
}

//BOOST_FIXTURE_TEST_CASE(transfer_fail_prohibiting_status1, HasInfoDomainDataWithDifferentInfoRegistrarDataAndServerTransferProhibited)
//{
//    BOOST_CHECK_THROW(
//        Epp::Domain::domain_transfer_impl(
//            ctx,
//            info_domain_data.fqdn,
//            info_domain_data.authinfopw,
//            different_info_registrar_data.id,
//            42
//        ),
//        Epp::ObjectStatusProhibitsOperation
//    );
//}

BOOST_FIXTURE_TEST_CASE(transfer_fail_autor_error, HasInfoDomainDataAndDifferentInfoRegistrarData)
{
    BOOST_CHECK_THROW(
        Epp::Domain::domain_transfer_impl(
            ctx,
            info_domain_data.fqdn,
            "thisisdifferent" + info_domain_data.authinfopw,
            different_info_registrar_data.id,
            42
        ),
        Epp::AuthorizationError
    );
}

struct HasInfoDomainDataWithServerUpdateProhibitedRequestAndDifferentInfoRegistrarData : HasDifferentInfoRegistrarData, HasInfoDomainDataWithServerUpdateProhibitedRequest { };

BOOST_FIXTURE_TEST_CASE(transfer_ok_state_requests_updated, HasInfoDomainDataWithServerUpdateProhibitedRequestAndDifferentInfoRegistrarData)
{
    Epp::Domain::domain_transfer_impl(
        ctx,
        info_domain_data.fqdn,
        info_domain_data.authinfopw,
        different_info_registrar_data.id,
        42
    );

    /* now object has the state server_update_prohibited itself */
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(info_domain_data.id).exec(ctx) ) {
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

BOOST_FIXTURE_TEST_CASE(transfer_ok_full_data, HasInfoDomainDataAndDifferentInfoRegistrarData)
{
    const Fred::InfoDomainData domain_data_before = Fred::InfoDomainByHandle(info_domain_data.fqdn).exec(ctx).info_domain_data;

    Epp::Domain::domain_transfer_impl(
        ctx,
        info_domain_data.fqdn,
        info_domain_data.authinfopw,
        different_info_registrar_data.id,
        42
    );

    const Fred::InfoDomainData domain_data_after = Fred::InfoDomainByHandle(info_domain_data.fqdn).exec(ctx).info_domain_data;

    const Fred::InfoDomainDiff domain_data_change = diff_domain_data(domain_data_before, domain_data_after);
    const std::set<std::string> change_fields_etalon = boost::assign::list_of
        ("sponsoring_registrar_handle")
        ("transfer_time")
        ("historyid")
        ("authinfopw");

    BOOST_CHECK(domain_data_change.changed_fields() == change_fields_etalon);

    BOOST_CHECK_EQUAL(domain_data_after.sponsoring_registrar_handle, different_info_registrar_data.handle);

    BOOST_CHECK_EQUAL(
        domain_data_after.transfer_time,
        boost::posix_time::time_from_string(
            static_cast<std::string>(
                ctx.get_conn().exec(
                    "SELECT now()::timestamp without time zone AT TIME ZONE 'UTC' AT TIME ZONE 'Europe/Prague' "
                )[0][0]
            )
        )
    );

    BOOST_CHECK(domain_data_before.historyid != domain_data_after.historyid);

    BOOST_CHECK(domain_data_before.authinfopw != domain_data_after.authinfopw);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
