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
#include "src/libfred/registrar/epp_auth/update_registrar_epp_auth.hh"
#include "src/util/password_storage.hh"
#include "src/util/password_storage/password_data.hh"
#include "src/util/random_data_generator.hh"
#include "test/libfred/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/optional.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <string>

namespace Test{

const std::string default_certificate = "cert_xxx";
const std::string default_password = "pass_xxx";

struct UpdateRegistrarEppAuthFixture : has_registrar
{
    unsigned long long id;
    std::string registrar_handle;
    std::string certificate_fingerprint;
    std::string plain_password;

    UpdateRegistrarEppAuthFixture()
        : certificate_fingerprint(default_certificate),
          plain_password(default_password)
    {
        registrar_handle =  registrar.handle;
        id = ::LibFred::Registrar::EppAuth::AddRegistrarEppAuth(
                registrar_handle, certificate_fingerprint, plain_password).exec(ctx);
    }
};

BOOST_FIXTURE_TEST_SUITE(TestUpdateRegistrarEppAuth, UpdateRegistrarEppAuthFixture)

size_t get_epp_auth_size(::LibFred::OperationContext& _ctx,
        const unsigned long long _id,
        const boost::optional<std::string>& _certificate_fingerprint,
        const boost::optional<std::string>& _plain_password)
{
    const std::string& certificate = _certificate_fingerprint.get_value_or(default_certificate);
    const std::string& password = _plain_password.get_value_or(default_password);

    const Database::Result db_result = _ctx.get_conn().exec_params(
            "SELECT password FROM registraracl "
            "WHERE id = $1::bigint "
            "AND cert = $2::text ",
            Database::query_param_list(_id)(certificate));

    if (_plain_password != boost::none && db_result.size() == 1)
    {
        const auto hashed_password =
                ::PasswordStorage::PasswordData::construct_from(static_cast<std::string>(db_result[0][0]));
        ::PasswordStorage::check_password(password, hashed_password);
    }
    return db_result.size();
}

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
    const std::string& new_certificate = RandomDataGenerator().xnstring(10);
    ::LibFred::Registrar::EppAuth::UpdateRegistrarEppAuth(id)
            .set_certificate_fingerprint(new_certificate)
            .set_plain_password(boost::none)
            .exec(ctx);
    BOOST_CHECK_EQUAL(get_epp_auth_size(ctx, id, new_certificate, boost::none), 1);
}

BOOST_AUTO_TEST_CASE(set_update_registrar_plain_password)
{
    const std::string& new_password = RandomDataGenerator().xnstring(10);
    ::LibFred::Registrar::EppAuth::UpdateRegistrarEppAuth(id)
            .set_plain_password(new_password)
            .exec(ctx);
    BOOST_CHECK_EQUAL(get_epp_auth_size(ctx, id, boost::none, new_password), 1);
}

BOOST_AUTO_TEST_CASE(set_update_registrar_all_epp_auth)
{
    const std::string& new_certificate = RandomDataGenerator().xnstring(10);
    const std::string& new_password = RandomDataGenerator().xnstring(10);
    ::LibFred::Registrar::EppAuth::UpdateRegistrarEppAuth(id)
            .set_certificate_fingerprint(new_certificate)
            .set_plain_password(new_password)
            .exec(ctx);
    BOOST_CHECK_EQUAL(get_epp_auth_size(ctx, id, new_certificate, new_password), 1);
}

BOOST_AUTO_TEST_SUITE_END();

} //namespace Test
