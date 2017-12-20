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

#ifndef UTIL_H_9DF076A0453C48F4BD81E5493BB289D3
#define UTIL_H_9DF076A0453C48F4BD81E5493BB289D3

#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include <boost/test/test_tools.hpp>

namespace Test {
namespace Fixture {

    struct autorollbacking_context : virtual instantiate_db_template {
        ::LibFred::OperationContextCreator ctx;
    };

} // namespace Test::Fixture
} // namespace Test

#endif
