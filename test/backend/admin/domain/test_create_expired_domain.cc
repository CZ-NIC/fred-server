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
#include "src/backend/admin/domain/create_expired_domain.hh"
#include "libfred/opcontext.hh"
#include "libfred/opexception.hh"
#include "test/backend/admin/domain/fixtures.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <map>
#include <set>
#include <string>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(Admin)
BOOST_AUTO_TEST_SUITE(Domain)
BOOST_AUTO_TEST_SUITE(TestAdminCreateExpiredDomain)

size_t exists_new_expired_domain(const std::string& _fqdn, const std::string& _registrant)
{
    LibFred::OperationContextCreator ctx;
    const Database::Result db_result = ctx.get_conn().exec_params(
        "SELECT 1 FROM object_registry AS od "
            "LEFT JOIN domain AS d ON d.id = od.id "
            "LEFT JOIN object_registry AS oc ON oc.id = d.registrant "
            "WHERE od.type = 3 "
                "AND od.name = $1::varchar "
                "AND od.erdate is null "
                "AND oc.name = UPPER($2::varchar) "
                "AND d.exdate = current_date ",
        Database::query_param_list(_fqdn)(_registrant));
    return db_result.size();
}

BOOST_FIXTURE_TEST_CASE(set_nonexistent_registrar, supply_fixture_ctx<HasNonexistentRegistrar>)
{
    BOOST_CHECK_THROW(
           ::Admin::Domain::create_expired_domain(*get_logger(), domain.fqdn, registrant.handle, cltrid, delete_existing, registrar.handle),
           ::Admin::Domain::SystemRegistrarNotExists
   );
}

BOOST_FIXTURE_TEST_CASE(set_nonsystem_registrar, supply_fixture_ctx<HasNonSystemRegistrar>)
{
    BOOST_CHECK_THROW(
           ::Admin::Domain::create_expired_domain(*get_logger(), domain.fqdn, registrant.handle, cltrid, delete_existing, registrar.handle),
           ::Admin::Domain::NotSystemRegistrar
   );
}

BOOST_FIXTURE_TEST_CASE(set_no_delete_existing_domain, supply_fixture_ctx<HasNoDeleteExistingDomain>)
{
    BOOST_CHECK_THROW(
           ::Admin::Domain::create_expired_domain(*get_logger(), domain.fqdn, registrant.handle, cltrid, delete_existing, registrar.handle),
           ::Admin::Domain::DomainExists
   );
}

BOOST_FIXTURE_TEST_CASE(set_nonexistent_regisrant, supply_fixture_ctx<HasNonexistentRegistrant>)
{
    BOOST_CHECK_THROW(
           ::Admin::Domain::create_expired_domain(*get_logger(), domain.fqdn, registrant.handle, cltrid, delete_existing, registrar.handle),
           ::Admin::Domain::RegistrantNotExists
   );
}

BOOST_FIXTURE_TEST_CASE(set_nonexistent_domain, supply_fixture_ctx<HasNonexistentDomain>)
{
    BOOST_CHECK_THROW(
           ::Admin::Domain::create_expired_domain(*get_logger(), domain.fqdn, registrant.handle, cltrid, delete_existing, registrar.handle),
           ::Admin::Domain::DomainNotExists
   );
}

BOOST_FIXTURE_TEST_CASE(set_existing_domain, supply_fixture_ctx<HasExistingDomain>)
{
   ::Admin::Domain::create_expired_domain(*get_logger(), domain.fqdn, registrant.handle, cltrid, delete_existing, registrar.handle);
   BOOST_CHECK_EQUAL(exists_new_expired_domain(domain.fqdn, registrant.handle), 1);
}

BOOST_FIXTURE_TEST_CASE(set_no_delete_nonexistent_domain, supply_fixture_ctx<HasNoDeleteNonexistentDomain>)
{
   ::Admin::Domain::create_expired_domain(*get_logger(), domain.fqdn, registrant.handle, cltrid, delete_existing, registrar.handle);
   BOOST_CHECK_EQUAL(exists_new_expired_domain(domain.fqdn, registrant.handle), 1);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
