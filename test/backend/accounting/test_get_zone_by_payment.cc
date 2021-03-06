/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#include "test/backend/accounting/util.hh"
#include "test/backend/accounting/fixture.hh"

#include "src/backend/accounting/impl/accounting.hh"
#include "src/backend/accounting/impl/exceptions.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

#include <set>
#include <vector>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Accounting)

BOOST_FIXTURE_TEST_SUITE(TestGetZoneByPayment, SupplyFixtureCtx<HasRegistrarsAndPayments>)

BOOST_AUTO_TEST_CASE(fail_registrar_not_found)
{
    BOOST_CHECK_THROW(
        ::Fred::Backend::Accounting::Impl::get_zone_by_payment(
            ctx,
            invalid_payment_data.data),
        ::Fred::Backend::Accounting::Impl::ZoneNotFound
    );
}

BOOST_AUTO_TEST_CASE(ok)
{
    BOOST_TEST_MESSAGE(payment_data.data.account_number);
    const std::string result =
            ::Fred::Backend::Accounting::Impl::get_zone_by_payment(
                    ctx,
                    payment_data.data);
    BOOST_TEST_MESSAGE(result);
    BOOST_CHECK_EQUAL(result, "cz");
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE_END(); // namespace Test::Backend::Accountin
BOOST_AUTO_TEST_SUITE_END(); // namespace Test::Backend

} // namespace Test
