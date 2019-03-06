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

#include "libfred/opcontext.hh"
#include "test/backend/admin_block/fixtures.hh"
#include "test/backend/admin_block/util.hh"
#include "test/setup/fixtures.hh"

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

namespace Test {

BOOST_AUTO_TEST_SUITE(Backend)
BOOST_AUTO_TEST_SUITE(AdminBlock)
BOOST_AUTO_TEST_SUITE(TestUnblockDomains)

BOOST_FIXTURE_TEST_CASE(set_domain_not_found, SupplyFixtureCtx<HasNonexistentDomain>)
{
    BOOST_CHECK_THROW(
           blocking_impl.unblockDomainsId(domain_list, new_owner.handle, remove_admin_c, reason, log_req_id),
           Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_NOT_FOUND
   );
}

BOOST_FIXTURE_TEST_CASE(set_domain_not_blocked, SupplyFixtureCtx<HasOneUnblockedDomain>)
{
    BOOST_CHECK_THROW(
           blocking_impl.unblockDomainsId(domain_list, new_owner.handle, remove_admin_c, reason, log_req_id),
           Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_NOT_BLOCKED
   );
}

BOOST_FIXTURE_TEST_CASE(set_no_domains, SupplyFixtureCtx<HasNoDomainForUnblock>)
{
    blocking_impl.unblockDomainsId(domain_list, new_owner.handle, remove_admin_c, reason, log_req_id);
    BOOST_CHECK_EQUAL(count_blocked_domains(domain_list, status_list), 0);
    BOOST_CHECK_EQUAL(count_blocked_domain_owners(domain_list, status_list), 0);
}

BOOST_FIXTURE_TEST_CASE(set_one_blocked_domain, SupplyFixtureCtx<HasOneBlockedDomain>)
{
    blocking_impl.blockDomainsId(domain_list, status_list, owner_block_mode, block_to_date, reason, log_req_id);
    BOOST_CHECK_EQUAL(count_blocked_domains(domain_list, status_list), domain_list.size());
    BOOST_CHECK_EQUAL(count_blocked_domain_owners(domain_list, status_list), 0);
    blocking_impl.unblockDomainsId(domain_list, new_owner.handle, remove_admin_c, reason, log_req_id);
    BOOST_CHECK_EQUAL(count_blocked_domains(domain_list, status_list), 0);
    BOOST_CHECK_EQUAL(count_blocked_domain_owners(domain_list, status_list), 0);
    BOOST_CHECK_EQUAL(count_blocked_origin_contacts(owner_list), 0);
    BOOST_CHECK_EQUAL(count_domains_with_admin_contacts(domain_list), domain_list.size());
    BOOST_CHECK_EQUAL(count_domains_with_user_blocking(domain_list), 0);
}

BOOST_FIXTURE_TEST_CASE(set_more_blocked_domains, SupplyFixtureCtx<HasMoreBlockedDomains>)
{
    blocking_impl.blockDomainsId(domain_list, status_list, owner_block_mode, block_to_date, reason, log_req_id);
    BOOST_CHECK_EQUAL(count_blocked_domains(domain_list, status_list), domain_list.size());
    BOOST_CHECK_EQUAL(count_blocked_domain_owners(domain_list, status_list), owner_list.size());
    blocking_impl.unblockDomainsId(domain_list, new_owner.handle, remove_admin_c, reason, log_req_id);
    BOOST_CHECK_EQUAL(count_blocked_domains(domain_list, status_list), 0);
    BOOST_CHECK_EQUAL(count_blocked_domain_owners(domain_list, status_list), 0);
    BOOST_CHECK_EQUAL(count_blocked_origin_contacts(owner_list), 0);
    BOOST_CHECK_EQUAL(count_domains_with_admin_contacts(domain_list), 0);
    BOOST_CHECK_EQUAL(count_domains_with_user_blocking(domain_list), 0);
}

BOOST_FIXTURE_TEST_CASE(set_more_owners_more_blocked_domains, SupplyFixtureCtx<HasMoreOwnersMoreBlockedDomains>)
{
    blocking_impl.blockDomainsId(domain_list, status_list, owner_block_mode, block_to_date, reason, log_req_id);
    BOOST_CHECK_EQUAL(count_blocked_domains(domain_list, status_list), domain_list.size());
    BOOST_CHECK_EQUAL(count_blocked_domain_owners(domain_list, status_list), owner_list.size());
    BOOST_CHECK_EQUAL(count_blocked_origin_contacts(owner_list), 0);
    blocking_impl.unblockDomainsId(some_domains, new_owner.handle, remove_admin_c, reason, log_req_id);
    BOOST_CHECK_EQUAL(count_blocked_domains(domain_list, status_list),
            domain_list.size() - some_domains.size());
    BOOST_CHECK_EQUAL(count_blocked_domain_owners(domain_list, status_list), owner_list.size());
    BOOST_CHECK_EQUAL(count_blocked_origin_contacts(owner_list), 0);
    BOOST_CHECK_EQUAL(count_domains_with_admin_contacts(domain_list),
            domain_list.size() - some_domains.size());
    BOOST_CHECK_EQUAL(count_domains_with_user_blocking(domain_list), 0);
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

} // namespace Test
