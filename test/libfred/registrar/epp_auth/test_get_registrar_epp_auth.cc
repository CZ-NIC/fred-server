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
#include "libfred/registrar/epp_auth/get_registrar_epp_auth.hh"
#include "libfred/registrar/epp_auth/registrar_epp_auth_data.hh"
#include "util/random_data_generator.hh"
#include "test/libfred/registrar/epp_auth/util.hh"
#include "test/libfred/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace Test{

struct GetRegistrarEppAuthFixture : has_registrar
{
    std::string& registrar_handle;
    std::string certificate_fingerprint;
    std::string plain_password;

    GetRegistrarEppAuthFixture()
        : registrar_handle(registrar.handle),
          certificate_fingerprint(RandomDataGenerator().xstring(20)),
          plain_password(RandomDataGenerator().xstring(10))
    {
    }
};

BOOST_FIXTURE_TEST_SUITE(TestGetRegistrarEppAuth, GetRegistrarEppAuthFixture)

BOOST_AUTO_TEST_CASE(set_nonexistent_registrar)
{
    const std::string nonexistent_registrar = RandomDataGenerator().xstring(10);
    ::LibFred::Registrar::EppAuth::RegistrarEppAuthData epp_auth_data =
            ::LibFred::Registrar::EppAuth::GetRegistrarEppAuth(nonexistent_registrar).exec(ctx);
    BOOST_CHECK_EQUAL(epp_auth_data.epp_auth_records.size(), 0);
}

BOOST_AUTO_TEST_CASE(set_nonexistent_registrar_epp_auth)
{
    ::LibFred::Registrar::EppAuth::RegistrarEppAuthData epp_auth_data =
            ::LibFred::Registrar::EppAuth::GetRegistrarEppAuth(registrar_handle).exec(ctx);
    BOOST_CHECK_EQUAL(epp_auth_data.epp_auth_records.size(), 0);
}

BOOST_AUTO_TEST_CASE(set_registrar_epp_authentication)
{
    add_epp_authentications(ctx, registrar_handle, certificate_fingerprint, plain_password);

    ::LibFred::Registrar::EppAuth::RegistrarEppAuthData epp_auth_data =
            ::LibFred::Registrar::EppAuth::GetRegistrarEppAuth(registrar_handle).exec(ctx);

    for (const auto& epp_auth : epp_auth_data.epp_auth_records)
    {
        BOOST_REQUIRE(get_epp_auth_id(ctx, registrar_handle, epp_auth.certificate_fingerprint, boost::none) > 0);
    }
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
