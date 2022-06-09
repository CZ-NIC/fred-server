/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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
#include "test/poc/parallel-tests/fixtures/contact.hh"
#include "test/poc/parallel-tests/fixtures/domain.hh"
#include "test/poc/parallel-tests/fixtures/has_fresh_database.hh"
#include "test/poc/parallel-tests/fixtures/keyset.hh"
#include "test/poc/parallel-tests/fixtures/nsset.hh"
#include "test/poc/parallel-tests/fixtures/operation_context.hh"
#include "test/poc/parallel-tests/fixtures/registrar.hh"
#include "test/poc/parallel-tests/fixtures/zone.hh"
#include "test/poc/parallel-tests/fixtures/zone_access.hh"

#include <boost/test/unit_test.hpp>

#include <utility>


namespace Test {

BOOST_AUTO_TEST_SUITE(ProofOfConcept)
BOOST_AUTO_TEST_SUITE(Parallel)
BOOST_AUTO_TEST_SUITE(Contact)

namespace {

struct RollbackingOperationContextFixture : HasFreshDatabase
{
    RollbackingOperationContextFixture()
        : HasFreshDatabase{},
          ctx{},
          registrar{ctx, Setter::registrar(LibFred::CreateRegistrar{"REG-A"})}
    { }
    OperationContext ctx;
    Registrar registrar;
};

struct CommittingOperationContextFixture : HasFreshDatabase
{
    CommittingOperationContextFixture()
        : HasFreshDatabase{},
          ctx{HasFreshDatabase::commit_done},
          cz_zone{ctx},
          commit{ctx},
          registrar{ctx, LibFred::CreateRegistrar{"REG-SYS-MY"}.set_system(true)
                                                               .set_name("My System Registrar")
                                                               .set_organization("My System Registrar Ltd")}
    { }
    OperationContext ctx;
    CzZone cz_zone;
    Commit commit;
    Registrar registrar;
};

struct HasCzZonesFixture : HasFreshDatabase
{
    HasCzZonesFixture()
        : HasFreshDatabase{},
          ctx{},
          cz_zone{ctx},
          cz_enum_zone{ctx},
          sys_registrar{ctx, Setter::system_registrar(LibFred::CreateRegistrar{"SYSREG-A"})},
          registrar{ctx, Setter::registrar(LibFred::CreateRegistrar{"REG-A"})},
          contact{ctx, Setter::contact(LibFred::CreateContact{"CONTACT", registrar.data.handle})},
          company{ctx, Setter::company(LibFred::CreateContact{"COMPANY", registrar.data.handle})},
          keyset{ctx, Setter::keyset(LibFred::CreateKeyset{"KEYSET", registrar.data.handle}.set_tech_contacts({contact.data.handle}))},
          nsset{ctx, Setter::nsset(LibFred::CreateNsset{"NSSET", registrar.data.handle}).set_tech_contacts({company.data.handle})},
          zone_access{ctx, cz_zone, registrar},
          domain{ctx, Setter::domain(LibFred::CreateDomain{CzZone::fqdn("nic"), registrar.data.handle, contact.data.handle})}
    { }
    OperationContext ctx;
    CzZone cz_zone;
    CzEnumZone cz_enum_zone;
    SystemRegistrar sys_registrar;
    Registrar registrar;
    Test::Contact contact;
    Test::Contact company;
    Keyset keyset;
    Nsset nsset;
    ZoneAccess zone_access;
    Domain domain;
};

}//namespace Test::{anonymous}

BOOST_FIXTURE_TEST_CASE(disabled_test, RollbackingOperationContextFixture, *boost::unit_test::enable_if<false>())
{
    BOOST_CHECK(false);
}

BOOST_FIXTURE_TEST_CASE(test_a, RollbackingOperationContextFixture)
{
    BOOST_CHECK_EQUAL(static_cast<std::string>(ctx.get_conn().exec("SELECT 'a'")[0][0]), "a");
    std::move(ctx).rollback_transaction();
}

BOOST_FIXTURE_TEST_CASE(test_b, CommittingOperationContextFixture)
{
    BOOST_CHECK_EQUAL(static_cast<std::string>(ctx.get_conn().exec("SELECT 'b'")[0][0]), "b");
    std::move(ctx).commit_transaction();
}

BOOST_FIXTURE_TEST_CASE(test_c, CommittingOperationContextFixture)
{
    BOOST_CHECK_EQUAL(static_cast<std::string>(ctx.get_conn().exec("SELECT 'c0'")[0][0]), "c0");
    ctx.commit_transaction();
    BOOST_CHECK_EQUAL(static_cast<std::string>(ctx.get_conn().exec("SELECT 'c1'")[0][0]), "c1");
    std::move(ctx).commit_transaction();
}

BOOST_FIXTURE_TEST_CASE(test_d, HasCzZonesFixture)
{
    BOOST_CHECK_EQUAL(static_cast<std::string>(ctx.get_conn().exec("SELECT 'd'")[0][0]), "d");
    Rollback rollback{std::move(ctx)};
    BOOST_CHECK_EQUAL(contact.data.handle, "CONTACT");
    BOOST_CHECK_EQUAL(company.data.handle, "COMPANY");
    BOOST_CHECK_EQUAL(domain.data.fqdn, "nic.cz");
}

BOOST_AUTO_TEST_CASE(test_cz_zones)
{
    BOOST_CHECK_EQUAL(CzZone::fqdn("nic"), "nic.cz");
    BOOST_CHECK_EQUAL(CzZone::fqdn(std::string{"seznam"}), "seznam.cz");
    BOOST_CHECK_EQUAL(CzZone::fqdn(std::string{"nic."}), "nic.cz");
    BOOST_CHECK_EQUAL(CzZone::fqdn("seznam."), "seznam.cz");
    BOOST_CHECK_EQUAL(CzEnumZone::fqdn(602), "2.0.6.0.2.4.e164.arpa");
}

BOOST_AUTO_TEST_SUITE_END()//ProofOfConcept/Parallel/Contact
BOOST_AUTO_TEST_SUITE_END()//ProofOfConcept/Parallel
BOOST_AUTO_TEST_SUITE_END()//ProofOfConcept

}//namespace Test
