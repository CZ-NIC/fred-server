/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#include "test/backend/automatic_keyset_management/util.hh"
#include "test/backend/automatic_keyset_management/fixture.hh"

#include "src/backend/automatic_keyset_management/automatic_keyset_management.hh"

#include "libfred/object_state/get_object_states.hh"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

#include <fstream>
#include <set>
#include <sstream>
#include <vector>

namespace Test {
namespace Backend {

BOOST_AUTO_TEST_SUITE(AutomaticKeysetmanagement)





BOOST_AUTO_TEST_SUITE_END();

} // namespace Test::Backend
} // namespace Test
