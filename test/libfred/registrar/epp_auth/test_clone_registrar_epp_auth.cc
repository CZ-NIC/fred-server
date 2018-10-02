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
#include "src/libfred/registrar/epp_auth/clone_registrar_epp_auth.hh"
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

struct CloneRegistrarEppAuthFixture : has_registrar
{
    std::string registrar_handle;
    std::string certificate_fingerprint;
    std::string plain_password;

    CloneRegistrarEppAuthFixture()
        : certificate_fingerprint(RandomDataGenerator().xstring(20)),
          plain_password(RandomDataGenerator().xstring(10))
    {
        registrar_handle =  registrar.handle;
    }
};

BOOST_FIXTURE_TEST_SUITE(TestCloneRegistrarEppAuth, CloneRegistrarEppAuthFixture)

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

BOOST_AUTO_TEST_CASE(set_nonexistent_registrar_epp_auth)
{
    const unsigned long long id = RandomDataGenerator().xuint();
    BOOST_CHECK_THROW(
            ::LibFred::Registrar::EppAuth::CloneRegistrarEppAuth(id, certificate_fingerprint).exec(ctx),
            ::LibFred::Registrar::EppAuth::NonexistentRegistrarEppAuth);
}

BOOST_AUTO_TEST_CASE(set_existing_certificate)
{
    const unsigned long long origin_id = ::LibFred::Registrar::EppAuth::AddRegistrarEppAuth(
            registrar_handle, certificate_fingerprint, plain_password).exec(ctx);
    BOOST_CHECK_THROW(
            ::LibFred::Registrar::EppAuth::CloneRegistrarEppAuth(origin_id, certificate_fingerprint).exec(ctx),
            ::LibFred::Registrar::EppAuth::DuplicateCertificate);
}

BOOST_AUTO_TEST_CASE(set_clone_registrar_epp_auth)
{
    const unsigned long long origin_id = ::LibFred::Registrar::EppAuth::AddRegistrarEppAuth(
            registrar_handle, certificate_fingerprint, plain_password).exec(ctx);
    const std::string& new_certificate = RandomDataGenerator().xstring(20);
    const unsigned long long new_id = ::LibFred::Registrar::EppAuth::CloneRegistrarEppAuth(
            origin_id, new_certificate).exec(ctx);
    BOOST_CHECK_EQUAL(get_epp_auth_id(ctx, registrar_handle, certificate_fingerprint, plain_password),
            origin_id);
    BOOST_CHECK_EQUAL(get_epp_auth_id(ctx, registrar_handle, new_certificate, plain_password), new_id);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
