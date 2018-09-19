/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

BOOST_FIXTURE_TEST_SUITE(TestGetRegistrarByHandle, SupplyFixtureCtx<HasRegistrarsAndPayments>)

BOOST_AUTO_TEST_CASE(fail_registrar_not_found)
{
    BOOST_CHECK_THROW(
        ::Fred::Backend::Accounting::Impl::get_registrar_by_handle(
            ctx,
            nonexistent_registrar_handle),
        ::Fred::Backend::Accounting::Impl::RegistrarNotFound
    );
}

BOOST_AUTO_TEST_CASE(ok)
{
    const ::Fred::Backend::Accounting::Registrar result =
            ::Fred::Backend::Accounting::Impl::get_registrar_by_handle(
                    ctx,
                    registrar.data.handle);
    BOOST_CHECK_EQUAL(result.handle, registrar.data.handle);
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE_END(); // namespace Test::Backend::Accountin
BOOST_AUTO_TEST_SUITE_END(); // namespace Test::Backend

} // namespace Test
