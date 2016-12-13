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

#include "src/epp/domain/domain_transfer_impl.h"
#include "tests/interfaces/epp/domain/fixture.h"
#include "tests/interfaces/epp/util.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(DomainTransferImpl)

BOOST_FIXTURE_TEST_CASE(fail_auth_error_srvr_closing_connection, HasInfoDomainData)
{
    BOOST_CHECK_THROW(
        Epp::Domain::domain_transfer_impl(
            ctx,
            info_domain_data_.fqdn,
            info_domain_data_.authinfopw,
            0,
            42
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(fail_nonexistent_handle, HasInfoRegistrarData)
{
    BOOST_CHECK_THROW(
        Epp::Domain::domain_transfer_impl(
            ctx,
            Test::get_nonexistent_object_handle(ctx),
            "abc-it-doesnt-matter-operation-should-fail-even-sooner",
            info_registrar_data_.id,
            42
        ),
        Epp::NonexistentHandle
    );
}

BOOST_FIXTURE_TEST_CASE(fail_not_eligible_for_transfer, HasInfoDomainData)
{
    BOOST_CHECK_THROW(
        Epp::Domain::domain_transfer_impl(
            ctx,
            info_domain_data_.fqdn,
            info_domain_data_.authinfopw,
            info_registrar_data_.id,
            42
        ),
        Epp::ObjectNotEligibleForTransfer
    );
}

//BOOST_FIXTURE_TEST_CASE(fail_prohibiting_status1, HasInfoDomainDataWithDifferentInfoRegistrarDataAndServerTransferProhibited)
//{
//    BOOST_CHECK_THROW(
//        Epp::Domain::domain_transfer_impl(
//            ctx,
//            info_domain_data_.fqdn,
//            info_domain_data_.authinfopw,
//            different_info_registrar_data_.id,
//            42
//        ),
//        Epp::ObjectStatusProhibitsOperation
//    );
//}

BOOST_FIXTURE_TEST_CASE(fail_authz_info_error, HasInfoDomainDataAndDifferentInfoRegistrarData)
{
    BOOST_CHECK_THROW(
        Epp::Domain::domain_transfer_impl(
            ctx,
            info_domain_data_.fqdn,
            "thisisdifferent" + info_domain_data_.authinfopw,
            different_info_registrar_data_.id,
            42
        ),
        Epp::AuthorizationInformationError
    );
}

BOOST_FIXTURE_TEST_CASE(fail_registrar_without_zone_access, HasInfoDomainDataWithInfoRegistrarDataOfRegistrarWithoutZoneAccess)
{
    BOOST_CHECK_THROW(
        Epp::Domain::domain_transfer_impl(
            ctx,
            info_domain_data_.fqdn,
            info_domain_data_.authinfopw,
            info_registrar_data_.id, // same registrar but zone access should be checked before this
            42
        ),
        Epp::AuthorizationError
    );
}

struct HasInfoDomainDataWithServerUpdateProhibitedRequestAndDifferentInfoRegistrarData : HasDifferentInfoRegistrarData, HasInfoDomainDataWithServerUpdateProhibitedRequest { };

BOOST_FIXTURE_TEST_CASE(ok_state_requests_updated, HasInfoDomainDataWithServerUpdateProhibitedRequestAndDifferentInfoRegistrarData)
{
    Epp::Domain::domain_transfer_impl(
        ctx,
        info_domain_data_.fqdn,
        info_domain_data_.authinfopw,
        different_info_registrar_data_.id,
        42
    );

    /* now object has the state server_update_prohibited itself */
    {
        std::vector<std::string> object_states_after;
        {
            BOOST_FOREACH(const Fred::ObjectStateData& state, Fred::GetObjectStates(info_domain_data_.id).exec(ctx) ) {
                object_states_after.push_back(state.state_name);
            }
        }

        BOOST_CHECK(
            std::find(object_states_after.begin(), object_states_after.end(), status_)
            !=
            object_states_after.end()
        );
    }
}

BOOST_FIXTURE_TEST_CASE(ok, HasInfoDomainDataAndDifferentInfoRegistrarData)
{
    const Fred::InfoDomainData domain_data_before = Fred::InfoDomainByHandle(info_domain_data_.fqdn).exec(ctx).info_domain_data;

    Epp::Domain::domain_transfer_impl(
        ctx,
        info_domain_data_.fqdn,
        info_domain_data_.authinfopw,
        different_info_registrar_data_.id,
        42
    );

    const Fred::InfoDomainData domain_data_after = Fred::InfoDomainByHandle(info_domain_data_.fqdn).exec(ctx).info_domain_data;

    const Fred::InfoDomainDiff domain_data_change = diff_domain_data(domain_data_before, domain_data_after);
    const std::set<std::string> change_fields_etalon = boost::assign::list_of
        ("sponsoring_registrar_handle")
        ("transfer_time")
        ("historyid")
        ("authinfopw");

    BOOST_CHECK(domain_data_change.changed_fields() == change_fields_etalon);

    BOOST_CHECK_EQUAL(domain_data_after.sponsoring_registrar_handle, different_info_registrar_data_.handle);

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

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
