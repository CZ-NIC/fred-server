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

#include "src/epp/domain/delete_domain.h"
#include "tests/interfaces/epp/domain/fixture.h"
#include "tests/interfaces/epp/util.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(DeleteDomain)

BOOST_FIXTURE_TEST_CASE(fail_invalid_registrar_id, HasInfoDomainData)
{
    BOOST_CHECK_THROW(
        Epp::Domain::delete_domain(
            ctx,
            info_domain_data_.fqdn,
            0 // invalid registrar_id
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(fail_domain_does_not_exist, HasInfoDomainDataOfNonexistentDomain)
{
    BOOST_CHECK_THROW(
        Epp::Domain::delete_domain(
            ctx,
            info_domain_data_.fqdn,
            info_registrar_data_.id
        ),
        Epp::ObjectDoesNotExist
    );
}

BOOST_FIXTURE_TEST_CASE(fail_enum_domain_does_not_exist, HasInfoDomainDataOfNonexistentEnumDomain)
{
    BOOST_CHECK_THROW(
        Epp::Domain::domain_delete_impl(
            ctx,
            info_enum_domain_data_.fqdn,
            info_registrar_data_.id
        ),
        Epp::ObjectDoesNotExist
    );
}

BOOST_FIXTURE_TEST_CASE(fail_wrong_registrar, HasInfoDomainDataAndDifferentInfoRegistrarData)
{
    BOOST_CHECK_THROW(
        Epp::Domain::delete_domain(
            ctx,
            info_domain_data_.fqdn,
            different_info_registrar_data_.id
        ),
        Epp::AuthorizationError
    );
}

BOOST_FIXTURE_TEST_CASE(fail_registrar_without_zone_access, HasInfoDomainDataWithInfoRegistrarDataOfRegistrarWithoutZoneAccess)
{
    BOOST_CHECK_THROW(
        Epp::Domain::delete_domain(
            ctx,
            info_domain_data_.fqdn,
            info_registrar_data_.id
        ),
        Epp::ZoneAuthorizationError
    );
}

BOOST_FIXTURE_TEST_CASE(fail_prohibiting_status, HasInfoDomainDataWithServerUpdateProhibited)
{
    BOOST_CHECK_THROW(
        Epp::Domain::delete_domain(
            ctx,
            info_domain_data_.fqdn,
            info_registrar_data_.id
        ),
        Epp::ObjectStatusProhibitsOperation
    );
}

BOOST_FIXTURE_TEST_CASE(ok, HasInfoDomainData)
{
    Epp::Domain::delete_domain(
        ctx,
        info_domain_data_.fqdn,
        info_registrar_data_.id
    );

    BOOST_CHECK_EQUAL(
        Fred::InfoDomainHistoryById(info_domain_data_.id).exec(ctx).rbegin()->info_domain_data.delete_time.isnull(),
        false
    );
}

BOOST_FIXTURE_TEST_CASE(ok_states_are_upgraded, HasInfoDomainDataWithServerTransferProhibited)
{
    Epp::Domain::delete_domain(
        ctx,
        info_domain_data_.fqdn,
        info_registrar_data_.id
    );

    /* now object has the state server_transfer_prohibited request itself */
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

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
