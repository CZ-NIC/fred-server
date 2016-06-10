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

#include "src/epp/nsset/nsset_create_impl.h"

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(NssetCreateImpl)

BOOST_AUTO_TEST_CASE( test_case_invalid_ip_unspecified )
{
    boost::system::error_code boost_error_code;//ignored purposefully, invalid ip address is transformed to unspecified
    BOOST_REQUIRE( boost::asio::ip::address::from_string("invalid_ip", boost_error_code).is_unspecified() );
}

BOOST_FIXTURE_TEST_CASE(create_invalid_registrar_id, has_nsset_input_data_set)
{
    BOOST_CHECK_THROW(
        Epp::nsset_create_impl(
            ctx,
            nsset_input_data,
            0 /* <== !!! */,
            42
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(create_fail_handle_format, has_nsset_input_data_set)
{
    nsset_input_data.handle +="?";
    try {
        Epp::nsset_create_impl(
            ctx,
            nsset_input_data,
            registrar.id,
            42 /* TODO */
        );
    } catch(...) {
        Test::check_correct_aggregated_exception_was_thrown( Epp::Error( Epp::Param::nsset_handle, 0, Epp::Reason::bad_format_nsset_handle ) );
    }
}

BOOST_FIXTURE_TEST_CASE(create_fail_already_existing, has_nsset_with_input_data_set)
{

    BOOST_CHECK_THROW(
        Epp::nsset_create_impl(
            ctx,
            nsset_input_data,
            registrar.id,
            42 /* TODO */
        ),
        Epp::ObjectExists
    );
}

BOOST_FIXTURE_TEST_CASE(create_fail_protected_handle, has_nsset_with_input_data_set)
{
    {   /* fixture */
        Fred::DeleteNssetByHandle(nsset.handle).exec(ctx);
    }

    try {
        Epp::nsset_create_impl(
            ctx,
            nsset_input_data,
            registrar.id,
            42 /* TODO */
        );
    } catch (...) {
        Test::check_correct_aggregated_exception_was_thrown( Epp::Error( Epp::Param::nsset_handle, 0, Epp::Reason::protected_period ) );
    }
}


bool boost_asio_ip_address_predicate (const boost::asio::ip::address& ip1, const boost::asio::ip::address& ip2)
{
    return (ip1 == ip2);
}

bool dnshostdata_dnshost_predicate (const Epp::DNShostData& dnshostdata, const Fred::DnsHost& dnshost)
{
    std::vector<boost::asio::ip::address> tmp = dnshost.get_inet_addr();
    return (dnshostdata.fqdn == dnshost.get_fqdn()
        && std::equal(dnshostdata.inet_addr.begin(), dnshostdata.inet_addr.end(), tmp.begin(), boost_asio_ip_address_predicate));
}

bool handle_oidhpair_predicate (const std::string& handle, const Fred::ObjectIdHandlePair& pair)
{
  return (handle == pair.handle);
}

void check_equal(const Epp::NssetCreateInputData& create_data, const Fred::InfoNssetData& info_data)
{
    BOOST_CHECK_EQUAL( boost::to_upper_copy( create_data.handle ), info_data.handle );
    BOOST_CHECK_EQUAL( create_data.authinfo, info_data.authinfopw );

    BOOST_CHECK_EQUAL(create_data.dns_hosts.size(), info_data.dns_hosts.size());
    BOOST_CHECK(std::equal (create_data.dns_hosts.begin(), create_data.dns_hosts.end(),
                info_data.dns_hosts.begin(), dnshostdata_dnshost_predicate));

    BOOST_CHECK_EQUAL(create_data.tech_contacts.size(), info_data.tech_contacts.size());
    BOOST_CHECK(std::equal (create_data.tech_contacts.begin(), create_data.tech_contacts.end(),
            info_data.tech_contacts.begin(), handle_oidhpair_predicate));

    BOOST_CHECK_EQUAL( create_data.tech_check_level , info_data.tech_check_level.get_value_or_default());
}

BOOST_FIXTURE_TEST_CASE(create_ok_all_data, has_nsset_input_data_set)
{
    const Epp::NssetCreateResult result = Epp::nsset_create_impl(
        ctx,
        nsset_input_data,
        registrar.id,
        42 /* TODO */
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
            check_sample
        );
    }
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
