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

#include "src/epp/domain/domain_create_impl.h"
#include "tests/interfaces/epp/domain/fixture.h"
#include "tests/interfaces/epp/util.h"

#include "src/epp/domain/domain_create.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(DomainCreateImpl)

BOOST_FIXTURE_TEST_CASE(create_invalid_registrar_id, HasDomainData)
{
    BOOST_CHECK_THROW(
        Epp::domain_create_impl(
            ctx,
            domain1_create_input_data,
            0,
            42
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(create_fail_already_existing, HasDomainData)
{

    BOOST_CHECK_THROW(
        Epp::domain_create_impl(
            ctx,
            domain2_create_input_data,
            info_registrar_data_.id,
            42 /* TODO */
        ),
        Epp::ObjectExists
    );
}

BOOST_FIXTURE_TEST_CASE(create_ok, HasDomainData)
{
    Epp::domain_create_impl(
        ctx,
        domain1_create_input_data,
        info_registrar_data_.id,
        42 /* TODO */
    );

    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(domain1_create_input_data.fqdn).exec(ctx).info_domain_data;
    BOOST_CHECK(info_data.fqdn == domain1_create_input_data.fqdn);

}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
