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

#include "src/epp/nsset/create_nsset.h"
#include "src/epp/nsset/impl/nsset.h"
#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <set>

BOOST_AUTO_TEST_SUITE(Nsset)
BOOST_AUTO_TEST_SUITE(CreateNsset)

BOOST_FIXTURE_TEST_CASE(test_case_uninitialized_ip_prohibited, has_registrar)
{
    boost::optional<boost::asio::ip::address> ip;
    BOOST_REQUIRE(Epp::Nsset::is_prohibited_ip_addr(ip,ctx));
}

bool create_invalid_registrar_id_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::authentication_error_server_closing_connection);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_invalid_registrar_id, has_nsset_input_data_set)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::create_nsset(
            ctx,
            nsset_input_data,
            nsset_config_data,
            0, // <== !!!
            42
        ),
        Epp::EppResponseFailure,
        create_invalid_registrar_id_exception
    );
}
bool create_fail_handle_format_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_syntax_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_REQUIRE(!e.epp_result().extended_errors()->empty());
    BOOST_CHECK(Epp::has_extended_error_with_param_reason(e.epp_result(), Epp::Param::nsset_handle, Epp::Reason::bad_format_nsset_handle));
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_fail_handle_format, has_nsset_input_data_set)
{
    nsset_input_data.handle +="?";
    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::create_nsset(
            ctx,
            nsset_input_data,
            nsset_config_data,
            registrar.id,
            42
        ),
        Epp::EppResponseFailure,
        create_fail_handle_format_exception
    );
}

bool create_fail_already_existing_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::object_exists);
    BOOST_CHECK(e.epp_result().empty());
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_fail_already_existing, has_nsset_with_input_data_set)
{
    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::create_nsset(
            ctx,
            nsset_input_data,
            nsset_config_data,
            registrar.id,
            42
        ),
        Epp::EppResponseFailure,
        create_fail_already_existing_exception
    );
}

bool create_fail_protected_handle_exception(const Epp::EppResponseFailure& e) {
    BOOST_CHECK_EQUAL(e.epp_result().epp_result_code(), Epp::EppResultCode::parameter_value_policy_error);
    BOOST_REQUIRE(e.epp_result().extended_errors());
    BOOST_REQUIRE(!e.epp_result().extended_errors()->empty());
    BOOST_CHECK(Epp::has_extended_error_with_param_reason(e.epp_result(), Epp::Param::nsset_handle, Epp::Reason::protected_period));
    return true;
}

BOOST_FIXTURE_TEST_CASE(create_fail_protected_handle, has_nsset_with_input_data_set)
{
    {   /* fixture */
        Fred::DeleteNssetByHandle(nsset.handle).exec(ctx);
    }

    BOOST_CHECK_EXCEPTION(
        Epp::Nsset::create_nsset(
            ctx,
            nsset_input_data,
            nsset_config_data,
            registrar.id,
            42
        ),
        Epp::EppResponseFailure,
        create_fail_protected_handle_exception
    );
}

bool boost_asio_ip_address_predicate (const boost::optional<boost::asio::ip::address>& ip1, const boost::asio::ip::address& ip2)
{
    return (ip1.is_initialized() && ip1.get() == ip2);
}

bool dnshostdata_dnshost_predicate (const Epp::Nsset::DnsHostInput& dnshostdata, const Fred::DnsHost& dnshost)
{
    std::vector<boost::asio::ip::address> tmp = dnshost.get_inet_addr();
    return (dnshostdata.fqdn == dnshost.get_fqdn()
        && std::equal(dnshostdata.inet_addr.begin(), dnshostdata.inet_addr.end(), tmp.begin(), boost_asio_ip_address_predicate));
}

bool handle_oidhpair_predicate (const std::string& handle, const Fred::ObjectIdHandlePair& pair)
{
  return (handle == pair.handle);
}

void check_equal(const Epp::Nsset::CreateNssetInputData& create_data, const Epp::Nsset::CreateNssetConfigData& config_data, const Fred::InfoNssetData& info_data)
{
    BOOST_CHECK_EQUAL( boost::to_upper_copy( create_data.handle ), info_data.handle );
    BOOST_CHECK_EQUAL( *create_data.authinfopw, info_data.authinfopw );

    BOOST_CHECK_EQUAL(create_data.dns_hosts.size(), info_data.dns_hosts.size());
    BOOST_CHECK(std::equal (create_data.dns_hosts.begin(), create_data.dns_hosts.end(),
                info_data.dns_hosts.begin(), dnshostdata_dnshost_predicate));

    BOOST_CHECK_EQUAL(create_data.tech_contacts.size(), info_data.tech_contacts.size());
    BOOST_CHECK(std::equal (create_data.tech_contacts.begin(), create_data.tech_contacts.end(),
            info_data.tech_contacts.begin(), handle_oidhpair_predicate));

    BOOST_CHECK_EQUAL( create_data.tech_check_level ? *create_data.tech_check_level : boost::numeric_cast<short>(config_data.default_tech_check_level), info_data.tech_check_level.get_value_or_default());
}

BOOST_FIXTURE_TEST_CASE(create_ok_all_data, has_nsset_input_data_set)
{
    const Epp::Nsset::CreateNssetResult result =
        Epp::Nsset::create_nsset(
            ctx,
            nsset_input_data,
            nsset_config_data,
            registrar.id,
            42
        );

    /* check returned data and db changes */
    {
        const Fred::InfoNssetData check_sample = Fred::InfoNssetByHandle(nsset_input_data.handle).exec(ctx).info_nsset_data;
        BOOST_CHECK_EQUAL( check_sample.id, result.id );
        BOOST_CHECK_EQUAL(
            boost::posix_time::time_from_string(
                static_cast<std::string>(
                    ctx.get_conn().exec("SELECT now() AT TIME ZONE 'utc' AS now_")[0]["now_"]
                )
            ),
            result.crdate
        );
        check_equal(
            nsset_input_data,
            nsset_config_data,
            check_sample
        );
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
