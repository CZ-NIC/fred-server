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

BOOST_FIXTURE_TEST_SUITE(TestGetRegistrarReferences, SupplyFixtureCtx<HasRegistrarsAndPayments>)

BOOST_AUTO_TEST_CASE(ok)
{
    std::vector<::Fred::Backend::Accounting::RegistrarReference> registrar_references =
            ::Fred::Backend::Accounting::Impl::get_registrar_references(
                    ctx);
    for (const auto& registrar_reference : registrar_references)
    {
        BOOST_TEST_MESSAGE(registrar_reference.name);
        BOOST_TEST_MESSAGE(registrar_reference.handle);
    }
    BOOST_CHECK(registrar_references.size() > 0);
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE_END(); // namespace Test::Backend::Accountin
BOOST_AUTO_TEST_SUITE_END(); // namespace Test::Backend

} // namespace Test

