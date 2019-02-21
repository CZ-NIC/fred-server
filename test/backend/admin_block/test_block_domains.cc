/*
 * Copyright (C) 2019  CZ.NIC, z.s.p.o.
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

#include "test/backend/admin_block/fixtures.hh"
#include "test/backend/admin_block/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(AdminBlock)
BOOST_AUTO_TEST_SUITE(TestBlockDomains)

BOOST_FIXTURE_TEST_CASE(set_no_domains, SupplyFixtureCtx<HasNoDomainForBlock>)
{
    blocking_impl.blockDomainsId(domain_list, status_list, owner_block_mode, block_to_date, reason, log_req_id);
    BOOST_CHECK_EQUAL(count_blocked_domains(domain_list, status_list), 0);
    BOOST_CHECK_EQUAL(count_blocked_domain_owners(domain_list, status_list), 0);
}

BOOST_FIXTURE_TEST_CASE(set_domain_not_found, SupplyFixtureCtx<HasNonexistentDomain>)
{
    BOOST_CHECK_THROW(
           blocking_impl.blockDomainsId(domain_list,
                   status_list, owner_block_mode, block_to_date, reason, log_req_id),
           Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_NOT_FOUND
   );
}

BOOST_FIXTURE_TEST_CASE(set_unknown_status, SupplyFixtureCtx<HasUknownStatus>)
{
    BOOST_CHECK_THROW(
           blocking_impl.blockDomainsId(domain_list,
                   status_list, owner_block_mode, block_to_date, reason, log_req_id),
           Fred::Backend::AdministrativeBlocking::EX_UNKNOWN_STATUS
   );
}

BOOST_FIXTURE_TEST_CASE(set_domain_already_blocked, SupplyFixtureCtx<HasOneDomain>)
{
    blocking_impl.blockDomainsId(domain_list, status_list, owner_block_mode, block_to_date, reason, log_req_id);
    BOOST_CHECK_THROW(
           blocking_impl.blockDomainsId(domain_list,
                   status_list, owner_block_mode, block_to_date, reason, log_req_id),
           Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_ALREADY_BLOCKED
   );
}

BOOST_FIXTURE_TEST_CASE(set_contact_block_prohibited, SupplyFixtureCtx<HasContactMojeId>)
{
    BOOST_CHECK_THROW(
           blocking_impl.blockDomainsId(domain_list,
                   status_list, owner_block_mode, block_to_date, reason, log_req_id),
           Fred::Backend::AdministrativeBlocking::EX_CONTACT_BLOCK_PROHIBITED
   );
}

BOOST_FIXTURE_TEST_CASE(set_owner_has_other_domain, SupplyFixtureCtx<HasMoreOwnersMoreDomains>)
{
    const auto other_owner_block_mode = Fred::Backend::AdministrativeBlocking::OWNER_BLOCK_MODE_BLOCK_OWNER;
    BOOST_CHECK_THROW(
           blocking_impl.blockDomainsId(domain_list,
                   status_list, other_owner_block_mode, block_to_date, reason, log_req_id),
           Fred::Backend::AdministrativeBlocking::EX_OWNER_HAS_OTHER_DOMAIN
   );
}

BOOST_FIXTURE_TEST_CASE(set_block_without_owner, SupplyFixtureCtx<HasOneDomain>)
{
    blocking_impl.blockDomainsId(domain_list, status_list, owner_block_mode, block_to_date, reason, log_req_id);
    BOOST_CHECK_EQUAL(count_blocked_domains(domain_list, status_list), domain_list.size());
    BOOST_CHECK_EQUAL(count_blocked_domain_owners(domain_list, status_list), 0);
}

BOOST_FIXTURE_TEST_CASE(set_block_with_owner, SupplyFixtureCtx<HasMoreDomains>)
{
    blocking_impl.blockDomainsId(domain_list, status_list, owner_block_mode, block_to_date, reason, log_req_id);
    BOOST_CHECK_EQUAL(count_blocked_domains(domain_list, status_list), domain_list.size());
    BOOST_CHECK_EQUAL(count_blocked_domain_owners(domain_list, status_list), owner_list.size());
}

BOOST_FIXTURE_TEST_CASE(set_block_copy_owners, SupplyFixtureCtx<HasMoreOwnersMoreDomains>)
{
    blocking_impl.blockDomainsId(domain_list, status_list, owner_block_mode, block_to_date, reason, log_req_id);
    BOOST_CHECK_EQUAL(count_blocked_domains(domain_list, status_list), domain_list.size());
    BOOST_CHECK_EQUAL(count_blocked_domain_owners(domain_list, status_list), owner_list.size());
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
