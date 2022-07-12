/*
 * Copyright (C) 2016-2022  CZ.NIC, z. s. p. o.
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

#include "test/backend/epp/util.hh"
#include "test/backend/epp/fixture.hh"
#include "test/backend/epp/nsset/fixture.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/epp_result_code.hh"
#include "src/backend/epp/nsset/dns_host_output.hh"
#include "src/backend/epp/nsset/info_nsset.hh"

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/optional.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Epp)
BOOST_AUTO_TEST_SUITE(Nsset)
BOOST_AUTO_TEST_SUITE(InfoNsset)

bool info_invalid_registrar_id_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(info_invalid_registrar_id, supply_ctx<HasSessionWithUnauthenticatedRegistrar>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Nsset::info_nsset(
                    ctx,
                    ValidHandle().handle,
                    DefaultInfoNssetConfigData(),
                    ::Epp::Password{},
                    session_with_unauthenticated_registrar.data),
            ::Epp::EppResponseFailure,
            info_invalid_registrar_id_exception);
}

bool info_fail_nonexistent_handle_exception(const ::Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(info_fail_nonexistent_handle, supply_ctx<HasRegistrarWithSessionAndNsset>)
{
    BOOST_CHECK_EXCEPTION(
            ::Epp::Nsset::info_nsset(
                    ctx,
                    NonexistentHandle().handle,
                    DefaultInfoNssetConfigData(),
                    ::Epp::Password{},
                    session.data),
            ::Epp::EppResponseFailure,
            info_fail_nonexistent_handle_exception);
}

static void check_equal(const ::Epp::Nsset::InfoNssetOutputData& nsset_data, const ::LibFred::InfoNssetData& info_data) {
    BOOST_CHECK_EQUAL( boost::to_upper_copy( nsset_data.handle ), info_data.handle );

    BOOST_CHECK_EQUAL( nsset_data.dns_hosts.size(), info_data.dns_hosts.size() );
    for(std::size_t i = 0; i < nsset_data.dns_hosts.size(); ++i)
    {
        BOOST_CHECK_EQUAL( nsset_data.dns_hosts.at(i).fqdn, info_data.dns_hosts.at(i).get_fqdn());

        BOOST_CHECK_EQUAL( nsset_data.dns_hosts.at(i).inet_addr.size(), info_data.dns_hosts.at(i).get_inet_addr().size() );
        for(std::size_t j = 0; j < nsset_data.dns_hosts.size(); ++j)
        {
            BOOST_CHECK_EQUAL( nsset_data.dns_hosts.at(i).inet_addr.at(j),info_data.dns_hosts.at(i).get_inet_addr().at(j));
        }
    }

    BOOST_CHECK_EQUAL( nsset_data.tech_contacts.size(), info_data.tech_contacts.size() );
    for(std::size_t i = 0; i < nsset_data.tech_contacts.size(); ++i)
    {
        BOOST_CHECK_EQUAL( nsset_data.tech_contacts.at(i), info_data.tech_contacts.at(i).handle );
    }

    BOOST_CHECK_EQUAL( nsset_data.tech_check_level, info_data.tech_check_level );
}

BOOST_FIXTURE_TEST_CASE(info_ok_full_data, supply_ctx<HasRegistrarWithSessionAndFullNsset>)
{
    check_equal(
        ::Epp::Nsset::info_nsset(
            ctx,
            nsset.data.handle,
            DefaultInfoNssetConfigData(),
            ::Epp::Password{},
            session.data
        ),
        nsset.data
    );
}

bool fail_invalid_authinfo(const ::Epp::EppResponseFailure& e)
{
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), ::Epp::EppResultCode::invalid_authorization_information);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(invalid_authinfo, supply_ctx<HasRegistrarWithSessionAndNssetWithAuthinfo>)
{
    BOOST_CHECK_EXCEPTION(
        ::Epp::Nsset::info_nsset(
            ctx,
            nsset.data.handle,
            DefaultInfoNssetConfigData(),
            ::Epp::Password{"invalid-" + *password},
            session.data
        ),
        ::Epp::EppResponseFailure,
        fail_invalid_authinfo);
}

BOOST_FIXTURE_TEST_CASE(authinfo_ok, supply_ctx<HasRegistrarWithSessionAndNssetWithAuthinfo>)
{
    check_equal(
        ::Epp::Nsset::info_nsset(
            ctx,
            nsset.data.handle,
            DefaultInfoNssetConfigData(),
            password,
            session.data
        ),
        nsset.data);
}

BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Nsset/InfoNsset
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp/Nsset
BOOST_AUTO_TEST_SUITE_END()//Backend/Epp
BOOST_AUTO_TEST_SUITE_END()//Backend

} // namespace Test
