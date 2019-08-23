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
#include "libfred/registrar/epp_auth/clone_registrar_epp_auth.hh"
#include "libfred/registrar/epp_auth/exceptions.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "test/libfred/registrar/epp_auth/util.hh"
#include "test/libfred/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace Test {

struct CloneRegistrarEppAuthFixture : has_registrar
{
    std::string& registrar_handle;
    std::string certificate_fingerprint;
    std::string plain_password;

    CloneRegistrarEppAuthFixture()
        : registrar_handle(registrar.handle),
          certificate_fingerprint(Random::Generator().get_seq(Random::CharSet::letters(), 20)),
          plain_password(Random::Generator().get_seq(Random::CharSet::letters(), 10))
    {
    }
};

BOOST_FIXTURE_TEST_SUITE(TestCloneRegistrarEppAuth, CloneRegistrarEppAuthFixture)

BOOST_AUTO_TEST_CASE(set_nonexistent_registrar_epp_auth)
{
    const unsigned long long id = Random::Generator().get(
            std::numeric_limits<unsigned>::min(),
            std::numeric_limits<unsigned>::max());
    BOOST_CHECK_THROW(
            ::LibFred::Registrar::EppAuth::CloneRegistrarEppAuth(id, certificate_fingerprint).exec(ctx),
            ::LibFred::Registrar::EppAuth::NonexistentRegistrarEppAuth);
}

BOOST_AUTO_TEST_CASE(set_existing_certificate)
{
    const unsigned long long id = add_epp_authentications(
            ctx, registrar_handle, certificate_fingerprint, plain_password);
    BOOST_CHECK_THROW(
            ::LibFred::Registrar::EppAuth::CloneRegistrarEppAuth(id, certificate_fingerprint).exec(ctx),
            ::LibFred::Registrar::EppAuth::DuplicateCertificate);
}

BOOST_AUTO_TEST_CASE(set_clone_registrar_epp_auth)
{
    const unsigned long long origin_id = add_epp_authentications(
            ctx, registrar_handle, certificate_fingerprint, plain_password);

    const std::string new_certificate = Random::Generator().get_seq(Random::CharSet::letters(), 20);
    const unsigned long long new_id = ::LibFred::Registrar::EppAuth::CloneRegistrarEppAuth(
            origin_id, new_certificate).exec(ctx);

    BOOST_CHECK_EQUAL(
            get_epp_auth_id(ctx, registrar_handle, certificate_fingerprint, plain_password), origin_id);
    BOOST_CHECK_EQUAL(get_epp_auth_id(ctx, registrar_handle, new_certificate, plain_password), new_id);
}

BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
