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
#include "libfred/registrar/epp_auth/delete_registrar_epp_auth.hh"
#include "libfred/registrar/epp_auth/exceptions.hh"
#include "util/random_data_generator.hh"
#include "test/libfred/registrar/epp_auth/util.hh"
#include "test/libfred/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace Test{

struct DeleteRegistrarEppAuthFixture : has_registrar
{
    std::string registrar_handle;
    std::string certificate_fingerprint;
    std::string plain_password;

    DeleteRegistrarEppAuthFixture()
        : registrar_handle(registrar.handle),
          certificate_fingerprint(RandomDataGenerator().xstring(20)),
          plain_password(RandomDataGenerator().xstring(10))
    {
    }
};

BOOST_FIXTURE_TEST_SUITE(TestDeleteRegistrarEppAuth, DeleteRegistrarEppAuthFixture)

BOOST_AUTO_TEST_CASE(set_nonexistent_registrar_epp_auth)
{
    const unsigned long long id = RandomDataGenerator().xuint();
    BOOST_CHECK_THROW(
            ::LibFred::Registrar::EppAuth::DeleteRegistrarEppAuth(id).exec(ctx),
            ::LibFred::Registrar::EppAuth::NonexistentRegistrarEppAuth);
}

BOOST_AUTO_TEST_CASE(set_delete_registrar_epp_auth)
{
    const unsigned long long id =
            add_epp_authentications(ctx, registrar_handle, certificate_fingerprint, plain_password);

    ::LibFred::Registrar::EppAuth::DeleteRegistrarEppAuth(id).exec(ctx);

    BOOST_CHECK_EQUAL(get_epp_auth_id(ctx, registrar_handle, certificate_fingerprint, plain_password), 0);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
