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

#include "tests/interfaces/epp/util.h"
#include "tests/interfaces/epp/nsset/fixture.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/nsset/dns_host_output.h"
#include "src/epp/nsset/info_nsset.h"

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/optional.hpp>

BOOST_AUTO_TEST_SUITE(Nsset)
BOOST_AUTO_TEST_SUITE(InfoNsset)

bool info_invalid_registrar_id_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(info_invalid_registrar_id, has_nsset)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::info_nsset(
            ctx,
            nsset.handle,
            Epp::SessionLang::en,
            0 /* <== !!! */
        ),
        Epp::EppResponseFailure,
        info_invalid_registrar_id_exception
    );
}

bool info_fail_nonexistent_handle_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_does_not_exist);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(info_fail_nonexistent_handle, has_nsset)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::info_nsset(
            ctx,
            nsset.handle + "SOMEobscureSTRING",
            Epp::SessionLang::en,
            42 /* TODO */
        ),
        Epp::EppResponseFailure,
        info_fail_nonexistent_handle_exception
    );
}

static void check_equal(const Epp::Nsset::InfoNssetOutputData& nsset_data, const Fred::InfoNssetData& info_data) {
    BOOST_CHECK_EQUAL( boost::to_upper_copy( nsset_data.handle ), info_data.handle );

    BOOST_REQUIRE(nsset_data.authinfopw);
    BOOST_CHECK_EQUAL( nsset_data.authinfopw.value(), info_data.authinfopw );

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

BOOST_FIXTURE_TEST_CASE(info_ok_full_data, has_nsset_with_all_data_set)
{
    check_equal(
        Epp::Nsset::info_nsset(
            ctx,
            nsset.handle,
            Epp::SessionLang::en,
            registrar.id
        ),
        nsset
    );
}

struct has_nsset_with_external_and_nonexternal_states : has_nsset {
    std::set<std::string> external_states;
    std::set<std::string> nonexternal_states;

    has_nsset_with_external_and_nonexternal_states() {
        external_states = boost::assign::list_of("serverDeleteProhibited")("serverUpdateProhibited").convert_to_container<std::set<std::string> >();
        nonexternal_states = boost::assign::list_of("deleteCandidate").convert_to_container<std::set<std::string> >();

        ctx.get_conn().exec_params(
            "UPDATE enum_object_states "
                "SET external = 'true'::bool, manual = 'true'::bool "
                "WHERE name = ANY( $1::text[] )",
            Database::query_param_list("{" + boost::algorithm::join(external_states, ", ") + "}")
        );

        ctx.get_conn().exec_params(
            "UPDATE enum_object_states "
                "SET external = 'false'::bool, manual = 'true'::bool "
                "WHERE name = ANY( $1::text[] )",
            Database::query_param_list("{" + boost::algorithm::join(nonexternal_states, ", ") + "}")
        );

        Fred::CreateObjectStateRequestId(nsset.id, external_states).exec(ctx);
        Fred::CreateObjectStateRequestId(nsset.id, nonexternal_states).exec(ctx);
        Fred::PerformObjectStateRequest(nsset.id).exec(ctx);

        ctx.commit_transaction();
    }
};

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
