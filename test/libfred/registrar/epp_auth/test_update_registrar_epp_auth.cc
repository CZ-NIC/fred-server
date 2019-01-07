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
#include "src/libfred/registrar/epp_auth/exceptions.hh"
#include "src/libfred/registrar/epp_auth/update_registrar_epp_auth.hh"
#include "src/util/random_data_generator.hh"
#include "test/libfred/registrar/epp_auth/util.hh"
#include "test/libfred/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/optional.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace Test{

namespace {
constexpr const char* default_certificate = "cert_xxx";
constexpr const char* default_password = "pass_xxx";
} //namespace Test::{anonymous}

struct UpdateRegistrarEppAuthFixture : has_registrar
{
    unsigned long long id;
    std::string& registrar_handle;
    std::string certificate_fingerprint;
    std::string plain_password;

    UpdateRegistrarEppAuthFixture()
        : registrar_handle(registrar.handle),
          certificate_fingerprint(default_certificate),
          plain_password(default_password)
    {
        id = add_epp_authentications(ctx, registrar_handle, certificate_fingerprint, plain_password);
    }
};

BOOST_FIXTURE_TEST_SUITE(TestUpdateRegistrarEppAuth, UpdateRegistrarEppAuthFixture)


BOOST_AUTO_TEST_CASE(set_no_update_data)
{
    BOOST_CHECK_THROW(
            ::LibFred::Registrar::EppAuth::UpdateRegistrarEppAuth(id).exec(ctx),
            ::LibFred::Registrar::EppAuth::NoUpdateData);
}

BOOST_AUTO_TEST_CASE(set_nonexistent_registrar_epp_auth)
{
    id = RandomDataGenerator().xuint();
    BOOST_CHECK_THROW(
            ::LibFred::Registrar::EppAuth::UpdateRegistrarEppAuth(id)
            .set_plain_password(plain_password)
            .exec(ctx),
            ::LibFred::Registrar::EppAuth::NonexistentRegistrarEppAuth);
}

BOOST_AUTO_TEST_CASE(set_update_registrar_certificate_fingerprint)
{
    const std::string new_certificate = RandomDataGenerator().xnstring(10);
    ::LibFred::Registrar::EppAuth::UpdateRegistrarEppAuth(id)
            .set_certificate_fingerprint(new_certificate)
            .set_plain_password(boost::none)
            .exec(ctx);
    BOOST_CHECK_EQUAL(get_epp_auth_id(ctx, registrar_handle, new_certificate, std::string(default_password)), id);
}

BOOST_AUTO_TEST_CASE(set_update_registrar_plain_password)
{
    const std::string new_password = RandomDataGenerator().xnstring(10);
    ::LibFred::Registrar::EppAuth::UpdateRegistrarEppAuth(id)
            .set_plain_password(new_password)
            .exec(ctx);
    BOOST_CHECK_EQUAL(get_epp_auth_id(ctx, registrar_handle, default_certificate, new_password), id);
}

BOOST_AUTO_TEST_CASE(set_update_registrar_all_epp_auth)
{
    const std::string new_certificate = RandomDataGenerator().xnstring(10);
    const std::string new_password = RandomDataGenerator().xnstring(10);
    ::LibFred::Registrar::EppAuth::UpdateRegistrarEppAuth(id)
            .set_certificate_fingerprint(new_certificate)
            .set_plain_password(new_password)
            .exec(ctx);
    BOOST_CHECK_EQUAL(get_epp_auth_id(ctx, registrar_handle, new_certificate, new_password), id);
}

BOOST_AUTO_TEST_SUITE_END();

} //namespace Test
