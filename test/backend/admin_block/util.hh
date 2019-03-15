/*
 * Copyright (C) 2019  CZ.NIC, z. s. p. o.
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
#ifndef UTIL_HH_D298DB00B71A4EA8967D55487C4E0449
#define UTIL_HH_D298DB00B71A4EA8967D55487C4E0449

#include "libfred/object_state/typedefs.hh"
#include "libfred/opcontext.hh"
#include "src/backend/admin_block/administrativeblocking.hh"
#include "test/setup/fixtures.hh"

namespace Test {

struct ContextHolder
    : virtual instantiate_db_template
{
    LibFred::OperationContextCreator ctx;
};

template <class T>
struct SupplyFixtureCtx : ContextHolder, T
{
    SupplyFixtureCtx()
        : ContextHolder(),
          T(ctx)
    {
        ctx.commit_transaction();
    }
};

std::size_t count_blocked_domains(const Fred::Backend::AdministrativeBlocking::IdlDomainIdList& _domain_list,
        const ::LibFred::StatusList& _status_list);

std::size_t count_blocked_domain_owners(const Fred::Backend::AdministrativeBlocking::IdlDomainIdList& _domain_list,
        const ::LibFred::StatusList& _status_list);

std::size_t count_blocked_origin_contacts(const std::set<std::pair<LibFred::ObjectId, std::string> >& _contact_list);

std::size_t count_domains_with_admin_contacts(
        const Fred::Backend::AdministrativeBlocking::IdlDomainIdList& _domain_list);

std::size_t count_domains_with_user_blocking(
        const Fred::Backend::AdministrativeBlocking::IdlDomainIdList& _domain_list);

} // namespace Test

#endif
