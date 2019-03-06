/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#ifndef FIXTURE_COMMON_HH_23091B98FDF94EE3B83A72ED232CDF75
#define FIXTURE_COMMON_HH_23091B98FDF94EE3B83A72ED232CDF75

#include "src/backend/whois/whois.hh"
#include "libfred/opcontext.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/exception/diagnostic_information.hpp>

struct whois_impl_instance_fixture : Test::instantiate_db_template
{
    Fred::Backend::Whois::Server_impl impl;

    whois_impl_instance_fixture()
    : impl("test-whois")
    {}
};

#endif
