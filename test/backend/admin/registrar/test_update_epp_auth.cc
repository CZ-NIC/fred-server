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
#include "src/backend/admin/registrar/update_epp_auth.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrar/epp_auth/get_registrar_epp_auth.hh"
#include "libfred/registrar/epp_auth/registrar_epp_auth_data.hh"
#include "test/backend/admin/registrar/fixtures.hh"
#include "test/libfred/registrar/epp_auth/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/optional.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <vector>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Admin)
BOOST_AUTO_TEST_SUITE(Registrar)
BOOST_AUTO_TEST_SUITE(TestUpdateEppAuth)

bool check_epp_auth(const ::Admin::Registrar::EppAuthData& _epp_auth_data)
{
    ::LibFred::OperationContextCreator ctx;
    for (const auto& auth : _epp_auth_data.epp_auth_records)
    {
        boost::optional<std::string> pass = boost::none;
        if (!auth.plain_password.empty())
        {
            pass = auth.plain_password;
        }
        Test::get_epp_auth_id(ctx, _epp_auth_data.registrar_handle, auth.certificate_fingerprint, pass);
        if (!auth.new_certificate_fingerprint.empty())
        {
            Test::get_epp_auth_id(ctx, _epp_auth_data.registrar_handle, auth.certificate_fingerprint, pass);
        }
    }
    return true;
}

BOOST_FIXTURE_TEST_CASE(set_noexistent_registrar, SupplyFixtureCtx<HasEppAuthWithNonexistentRegistrar>)
{
    BOOST_CHECK_THROW(::Admin::Registrar::update_epp_auth(epp_auth_data),
            ::Admin::Registrar::EppAuthNonexistentRegistrar);

    const std::string reg_handle = epp_auth_data.registrar_handle;
    const std::string certificate = "new certificate";
    const std::string password = "new password";
    BOOST_CHECK_THROW(::Admin::Registrar::add_epp_auth(reg_handle, certificate, password),
            ::Admin::Registrar::EppAuthNonexistentRegistrar);
}

BOOST_FIXTURE_TEST_CASE(set_missing_params, SupplyFixtureCtx<HasEppAuthMissingParams>)
{
    BOOST_CHECK_THROW(::Admin::Registrar::update_epp_auth(epp_auth_data),
            ::Admin::Registrar::EppAuthMissingParameters);
}

BOOST_FIXTURE_TEST_CASE(set_add_duplicate_cert, SupplyFixtureCtx<HasAddDuplicateCert>)
{
    BOOST_CHECK_THROW(::Admin::Registrar::update_epp_auth(epp_auth_data),
            ::Admin::Registrar::DuplicateCertificate);

    const ::Admin::Registrar::EppAuthRecord new_auth = epp_auth_data.epp_auth_records[0];
    BOOST_CHECK_THROW(::Admin::Registrar::add_epp_auth(epp_auth_data.registrar_handle,
                   new_auth.certificate_fingerprint,
                   new_auth.plain_password),
            ::Admin::Registrar::DuplicateCertificate);
}

BOOST_FIXTURE_TEST_CASE(set_update_duplicate_cert, SupplyFixtureCtx<HasUpdateDuplicateCert>)
{
    BOOST_CHECK_THROW(::Admin::Registrar::update_epp_auth(epp_auth_data),
            ::Admin::Registrar::DuplicateCertificate);
}

BOOST_FIXTURE_TEST_CASE(set_clone_duplicate_cert, SupplyFixtureCtx<HasCloneDuplicateCert>)
{
    BOOST_CHECK_THROW(::Admin::Registrar::update_epp_auth(epp_auth_data),
            ::Admin::Registrar::DuplicateCertificate);
}

BOOST_FIXTURE_TEST_CASE(set_no_update_data, SupplyFixtureCtx<HasEppAuthNoUpdateData>)
{
    BOOST_CHECK_THROW(::Admin::Registrar::update_epp_auth(epp_auth_data),
            ::Admin::Registrar::EppAuthNoUpdateData);
}

BOOST_FIXTURE_TEST_CASE(set_nonexistent_registrar_epp_auth, SupplyFixtureCtx<HasNoexistentEppAuth>)
{
    BOOST_CHECK_THROW(::Admin::Registrar::update_epp_auth(epp_auth_data),
            ::Admin::Registrar::NonexistentEppAuth);
}

BOOST_FIXTURE_TEST_CASE(set_empty_epp_auth, SupplyFixtureCtx<HasEmptyEppAuth>)
{
    ::Admin::Registrar::update_epp_auth(epp_auth_data);
    BOOST_CHECK(check_epp_auth(epp_auth_data));
}

BOOST_FIXTURE_TEST_CASE(set_add_epp_auth, SupplyFixtureCtx<HasEppAuthAddWithDelete>)
{
    ::Admin::Registrar::update_epp_auth(epp_auth_data);

    ::Admin::Registrar::EppAuthRecord new_auth;
    new_auth.certificate_fingerprint = registrar.handle + "new certificate";
    new_auth.plain_password = registrar.handle + "new password";
    ::Admin::Registrar::add_epp_auth(registrar.handle,
                   new_auth.certificate_fingerprint,
                   new_auth.plain_password);
    epp_auth_data.epp_auth_records.push_back(new_auth);

    BOOST_CHECK(check_epp_auth(epp_auth_data));
}

BOOST_FIXTURE_TEST_CASE(set_update_epp_auth, SupplyFixtureCtx<HasEppAuthUpdateWithDelete>)
{
    ::Admin::Registrar::update_epp_auth(epp_auth_data);
    BOOST_CHECK(check_epp_auth(epp_auth_data));
}

BOOST_FIXTURE_TEST_CASE(set_clone_epp_auth, SupplyFixtureCtx<HasEppAuthCloneWithDelete>)
{
    ::Admin::Registrar::update_epp_auth(epp_auth_data);
    BOOST_CHECK(check_epp_auth(epp_auth_data));
}

BOOST_FIXTURE_TEST_CASE(set_more_epp_auth, SupplyFixtureCtx<HasMoreEppAuthData>)
{
    ::Admin::Registrar::update_epp_auth(epp_auth_data);
    BOOST_CHECK(check_epp_auth(epp_auth_data));
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
