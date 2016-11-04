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

#include "src/epp/domain/domain_renew_impl.h"
#include "tests/interfaces/epp/domain/fixture.h"
#include "tests/interfaces/epp/util.h"

#include "src/epp/domain/domain_renew.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestEpp)
BOOST_AUTO_TEST_SUITE(DomainRenewImpl)

BOOST_FIXTURE_TEST_CASE(renew_invalid_registrar_id, HasDomainData)
{
    BOOST_CHECK_THROW(
        Epp::domain_renew_impl(
            ctx,
            domain2_renew_input_data.value(),
            0,
            42
        ),
        Epp::AuthErrorServerClosingConnection
    );
}

BOOST_FIXTURE_TEST_CASE(renew_invalid_fqdn, HasDomainData)
{
    BOOST_CHECK_THROW(
        Epp::domain_renew_impl(
            ctx,
            domain1_renew_input_data,
            0,
            42
        ),
        Epp::ObjectDoesNotExist
    );
}

BOOST_FIXTURE_TEST_CASE(renew_ok, HasDomainData)
{
    Epp::DomainRenewResult renew_result =
    Epp::domain_renew_impl(
        ctx,
        domain2_renew_input_data.value(),
        info_registrar_data_.id,
        42
    );

    Fred::InfoDomainData info_data = Fred::InfoDomainByHandle(domain2_renew_input_data.value().fqdn).exec(ctx).info_domain_data;
    BOOST_CHECK(info_data.expiration_date == renew_result.exdate);

}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
