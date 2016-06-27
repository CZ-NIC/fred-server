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

#include "tests/interfaces/epp/keyset/fixture.h"
#include "src/epp/keyset/create.h"
#include "src/fredlib/registrar/create_registrar.h"

#include <sstream>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(Keyset)

BOOST_FIXTURE_TEST_CASE(create, Test::ObjectsProvider)
{
    const std::string registrar_handle = this->get_registrar_a().handle;
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END();
