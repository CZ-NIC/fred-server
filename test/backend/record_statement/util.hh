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

/**
 *  @file
 */

#ifndef TEST_INTERFACE_RECORD_STATEMENT_UTIL_6B02C7FC0F294408B0C9C218355808A0
#define TEST_INTERFACE_RECORD_STATEMENT_UTIL_6B02C7FC0F294408B0C9C218355808A0

#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include <boost/test/test_tools.hpp>

namespace Test {

    struct autorollbacking_context : virtual instantiate_db_template {
        ::LibFred::OperationContextCreator ctx;
    };

} // namespace Test

#endif
