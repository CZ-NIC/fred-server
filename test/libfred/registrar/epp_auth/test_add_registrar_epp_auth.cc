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
#include "src/libfred/db_settings.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/registrar/epp_auth/add_registrar_epp_auth.hh"
#include "src/libfred/registrar/epp_auth/exceptions.hh"
#include "src/util/password_storage.hh"
#include "src/util/password_storage/password_data.hh"
#include "src/util/random_data_generator.hh"
#include "test/libfred/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace Test {

struct AddRegistrarEppAuthFixture : has_registrar
{
    std::string registrar_handle;
    std::string certificate_fingerprint;
    std::string plain_password;

    AddRegistrarEppAuthFixture()
        : certificate_fingerprint(RandomDataGenerator().xstring(20)),
          plain_password(RandomDataGenerator().xstring(10))
    {
        registrar_handle =  registrar.handle;
    }
};

BOOST_FIXTURE_TEST_SUITE(TestAddRegistrarEppAuth, AddRegistrarEppAuthFixture)

unsigned long long get_epp_auth_id(::LibFred::OperationContext& _ctx,
        const std::string& _registrar_handle,
        const std::string& _certificate_fingerprint,
        const std::string& _plain_password)
{
    const Database::Result db_result = _ctx.get_conn().exec_params(
            "SELECT ra.id, ra.password FROM registraracl AS ra "
            "JOIN registrar AS r ON r.id=ra.registrarid "
            "WHERE r.handle = $1::text "
            "AND ra.cert = $2::text ",
            Database::query_param_list(_registrar_handle)
                    (_certificate_fingerprint));
    if (db_result.size() == 1)
    {
        const auto id = static_cast<unsigned long long>(db_result[0][0]);
        const auto hashed_password =
                ::PasswordStorage::PasswordData::construct_from(static_cast<std::string>(db_result[0][1]));
        ::PasswordStorage::check_password(_plain_password, hashed_password);
        return id;
    }
    throw ;
}

BOOST_AUTO_TEST_CASE(set_nonexistent_registrar)
{
    registrar_handle = RandomDataGenerator().xstring(20);
    BOOST_CHECK_THROW(
            ::LibFred::Registrar::EppAuth::AddRegistrarEppAuth(
                    registrar_handle, certificate_fingerprint, plain_password).exec(ctx),
            ::LibFred::Registrar::EppAuth::NonexistentRegistrar);
}

BOOST_AUTO_TEST_CASE(set_add_registrar_epp_auth)
{
    const unsigned long long id = ::LibFred::Registrar::EppAuth::AddRegistrarEppAuth(
            registrar_handle, certificate_fingerprint, plain_password).exec(ctx);
    BOOST_CHECK_EQUAL(get_epp_auth_id(ctx, registrar_handle, certificate_fingerprint, plain_password), id);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
