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
#ifndef FIXTURES_HH_1D30A33D2290443D9DD68F21A7CFEA64
#define FIXTURES_HH_1D30A33D2290443D9DD68F21A7CFEA64

#include "libfred/object/object_state.hh"
#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/object_state/typedefs.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "src/backend/admin_block/administrativeblocking.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"
#include "util/random/random.hh"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <limits>
#include <set>
#include <string>

namespace Test {
namespace Backend {
namespace AdminBlock {

struct NewContact
{
    Nullable<std::string> handle;
    NewContact(LibFred::OperationContext& _ctx, const std::string _registrar = "")
    {
        std::string reg;
        if (_registrar.empty())
        {
            reg = Test::registrar::make(_ctx).handle;
        }
        else
        {
            reg = _registrar;
        }
        handle = Test::exec(Test::generate_test_data(
                        Test::CreateX_factory< ::LibFred::CreateContact>().make(reg)), _ctx).handle;
    }
};

struct DomainList
{
    Fred::Backend::AdministrativeBlocking::IdlDomainIdList domain_list;
    Fred::Backend::AdministrativeBlocking::IdlDomainIdList some_domains;
    std::set<std::pair<LibFred::ObjectId, std::string> > owner_list;
    ::LibFred::StatusList user_blocking_states;

    DomainList(LibFred::OperationContext& _ctx, std::size_t _list_size, bool _more_owners = false)
    {
        std::string registrar = Test::registrar::make(_ctx).handle;
        NewContact contact = NewContact(_ctx, registrar);
        NewContact admin_contact = NewContact(_ctx, registrar);

        user_blocking_states.insert(Conversion::Enums::to_db_handle(
                        LibFred::Object_State::server_transfer_prohibited));
        user_blocking_states.insert(Conversion::Enums::to_db_handle(
                        LibFred::Object_State::server_update_prohibited));

        for (std::size_t i = 0; i < _list_size; ++i)
        {
            if (_more_owners && i%3 == 0)
            {
                 contact = NewContact(_ctx, registrar);
            }
            LibFred::InfoDomainData domain = Test::exec(Test::CreateX_factory<LibFred::CreateDomain>()
                            .make(registrar, contact.handle.get_value()), _ctx);
            ::LibFred::UpdateDomain(domain.fqdn, registrar)
                    .add_admin_contact(admin_contact.handle.get_value())
                    .exec(_ctx);
            domain_list.insert(domain.id);
            owner_list.insert(make_pair(domain.registrant.id, contact.handle.get_value()));
            ::LibFred::CreateObjectStateRequestId(domain.id, user_blocking_states).exec(_ctx);
            ::LibFred::PerformObjectStateRequest(domain.id).exec(_ctx);
            if (_more_owners && i%4 == 0)
            {
                 some_domains.insert(domain.id);
            }
        }
    }
};

struct StatusList
{
    ::LibFred::StatusList status_list;

    StatusList(LibFred::OperationContext& _ctx, bool _all_administrative_states = false)
    {
        status_list.insert(Conversion::Enums::to_db_handle(LibFred::Object_State::server_delete_prohibited));
        if (_all_administrative_states)
        {
            status_list.insert(Conversion::Enums::to_db_handle(
                            LibFred::Object_State::server_renew_prohibited));
            status_list.insert(Conversion::Enums::to_db_handle(
                            LibFred::Object_State::server_transfer_prohibited));
            status_list.insert(Conversion::Enums::to_db_handle(
                            LibFred::Object_State::server_update_prohibited));
            status_list.insert(Conversion::Enums::to_db_handle(
                            LibFred::Object_State::server_outzone_manual));
            status_list.insert(Conversion::Enums::to_db_handle(
                            LibFred::Object_State::server_registrant_change_prohibited));
        }
    }
};

struct HasNoDomainForBlock
{
    Fred::Backend::AdministrativeBlocking::BlockingImpl blocking_impl;
    Fred::Backend::AdministrativeBlocking::IdlDomainIdList domain_list;
    ::LibFred::StatusList status_list;
    Fred::Backend::AdministrativeBlocking::IdlOwnerBlockMode owner_block_mode;
    Nullable<boost::gregorian::date> block_to_date;
    std::string reason;
    unsigned long long log_req_id;

    HasNoDomainForBlock(LibFred::OperationContext& _ctx)
            : blocking_impl("test_server"),
              domain_list(),
              status_list(),
              owner_block_mode(),
              block_to_date(),
              reason("No domains for block")
    {
    }
};

struct HasNonexistentDomain : DomainList, StatusList
{
    Fred::Backend::AdministrativeBlocking::BlockingImpl blocking_impl;
    Fred::Backend::AdministrativeBlocking::IdlOwnerBlockMode owner_block_mode;
    Nullable<boost::gregorian::date> block_to_date;
    NewContact new_owner;
    bool remove_admin_c;
    std::string reason;
    unsigned long long log_req_id;

    HasNonexistentDomain(LibFred::OperationContext& _ctx)
            : DomainList(_ctx, 1),
              StatusList(_ctx),
              blocking_impl("test_server"),
              owner_block_mode(),
              block_to_date(),
              new_owner(_ctx),
              remove_admin_c(false),
              reason("Nonexistent domain")
    {
        const auto nonexistent_domain_id = Test::get_nonexistent_object_id(_ctx);
        domain_list.insert(nonexistent_domain_id);
    }
};

struct HasUknownStatus : DomainList, StatusList
{
    Fred::Backend::AdministrativeBlocking::BlockingImpl blocking_impl;
    Fred::Backend::AdministrativeBlocking::IdlOwnerBlockMode owner_block_mode;
    Nullable<boost::gregorian::date> block_to_date;
    std::string reason;
    unsigned long long log_req_id;

    HasUknownStatus(LibFred::OperationContext& _ctx)
            : DomainList(_ctx, 1),
              StatusList(_ctx),
              blocking_impl("test_server"),
              owner_block_mode(),
              block_to_date(),
              reason("UknownStatus")
    {
        const std::string state = Test::get_nonexistent_value(_ctx,
                "enum_object_states", "name", "text", generate_random_handle);
        status_list.insert(state);
    }
};

struct HasContactMojeId : DomainList, StatusList
{
    Fred::Backend::AdministrativeBlocking::BlockingImpl blocking_impl;
    Fred::Backend::AdministrativeBlocking::IdlOwnerBlockMode owner_block_mode;
    Nullable<boost::gregorian::date> block_to_date;
    std::string reason;
    unsigned long long log_req_id;

    HasContactMojeId(LibFred::OperationContext& _ctx)
            : DomainList(_ctx, 1),
              StatusList(_ctx),
              blocking_impl("test_server"),
              owner_block_mode(::Fred::Backend::AdministrativeBlocking::OWNER_BLOCK_MODE_BLOCK_OWNER),
              block_to_date(),
              reason("ContactMojeId"),
              log_req_id(Random::Generator().get(
                          std::numeric_limits<unsigned>::min(), 
                          std::numeric_limits<unsigned>::max()))
    {
        const auto owner = owner_list.begin();
        ::LibFred::StatusList status_moje_id;
        status_moje_id.insert(Conversion::Enums::to_db_handle(::LibFred::Object_State::mojeid_contact));
        ::LibFred::CreateObjectStateRequestId(owner->first, status_moje_id).exec(_ctx);
        ::LibFred::PerformObjectStateRequest(owner->first).exec(_ctx);
    }
};

struct HasOneDomain : DomainList, StatusList
{
    Fred::Backend::AdministrativeBlocking::BlockingImpl blocking_impl;
    Fred::Backend::AdministrativeBlocking::IdlOwnerBlockMode owner_block_mode;
    Nullable<boost::gregorian::date> block_to_date;
    std::string reason;
    unsigned long long log_req_id;

    HasOneDomain(LibFred::OperationContext& _ctx)
            : DomainList(_ctx, 1),
              StatusList(_ctx),
              blocking_impl("test_server"),
              owner_block_mode(::Fred::Backend::AdministrativeBlocking::OWNER_BLOCK_MODE_KEEP_OWNER),
              block_to_date(),
              reason("One Domain"),
              log_req_id(Random::Generator().get(
                          std::numeric_limits<unsigned>::min(), 
                          std::numeric_limits<unsigned>::max()))
    {
    }
};

struct HasMoreDomains : DomainList, StatusList
{
    Fred::Backend::AdministrativeBlocking::BlockingImpl blocking_impl;
    Fred::Backend::AdministrativeBlocking::IdlOwnerBlockMode owner_block_mode;
    Nullable<boost::gregorian::date> block_to_date;
    std::string reason;
    unsigned long long log_req_id;

    HasMoreDomains(LibFred::OperationContext& _ctx)
            : DomainList(_ctx, 5),
              StatusList(_ctx),
              blocking_impl("test_server"),
              owner_block_mode(::Fred::Backend::AdministrativeBlocking::OWNER_BLOCK_MODE_BLOCK_OWNER),
              block_to_date(),
              reason("More Domains"),
              log_req_id(Random::Generator().get(
                          std::numeric_limits<unsigned>::min(), 
                          std::numeric_limits<unsigned>::max()))
    {
    }
};

struct HasMoreOwnersMoreDomains : DomainList, StatusList
{
    Fred::Backend::AdministrativeBlocking::BlockingImpl blocking_impl;
    Fred::Backend::AdministrativeBlocking::IdlOwnerBlockMode owner_block_mode;
    Nullable<boost::gregorian::date> block_to_date;
    std::string reason;
    unsigned long long log_req_id;

    HasMoreOwnersMoreDomains(LibFred::OperationContext& _ctx)
            : DomainList(_ctx, 10, true),
              StatusList(_ctx, true),
              blocking_impl("test_server"),
              owner_block_mode(::Fred::Backend::AdministrativeBlocking::OWNER_BLOCK_MODE_BLOCK_OWNER_COPY),
              block_to_date(),
              reason("MoreOwnersMoreDomains"),
              log_req_id(Random::Generator().get(
                          std::numeric_limits<unsigned>::min(), 
                          std::numeric_limits<unsigned>::max()))
    {
        for (const auto& owner : owner_list)
        {
            LibFred::InfoDomainData domain =
                    Test::exec(Test::CreateX_factory<LibFred::CreateDomain>()
                            .make(Test::registrar::make(_ctx).handle, owner.second), _ctx);
        }
    }
};

struct HasNoDomainForUnblock : StatusList
{
    Fred::Backend::AdministrativeBlocking::IdlDomainIdList domain_list;
    Fred::Backend::AdministrativeBlocking::BlockingImpl blocking_impl;
    NewContact new_owner;
    bool remove_admin_c;
    std::string reason;
    unsigned long long log_req_id;

    HasNoDomainForUnblock(LibFred::OperationContext& _ctx)
            : StatusList(_ctx),
              domain_list(),
              blocking_impl("test_server"),
              new_owner(_ctx),
              remove_admin_c(false),
              reason("No domains for unblock")
    {
    }
};

struct HasOneUnblockedDomain : HasOneDomain
{
    NewContact new_owner;
    bool remove_admin_c;

    HasOneUnblockedDomain(LibFred::OperationContext& _ctx)
            : HasOneDomain(_ctx),
              new_owner(_ctx),
              remove_admin_c(false)
    {
    }
};

struct HasOneBlockedDomain : HasOneDomain
{
    NewContact new_owner;
    bool remove_admin_c;

    HasOneBlockedDomain(LibFred::OperationContext& _ctx)
            : HasOneDomain(_ctx),
              new_owner(_ctx),
              remove_admin_c(false)
    {
    }
};

struct HasMoreBlockedDomains : HasMoreDomains
{
    NewContact new_owner;
    bool remove_admin_c;

    HasMoreBlockedDomains(LibFred::OperationContext& _ctx)
            : HasMoreDomains(_ctx),
              new_owner(_ctx),
              remove_admin_c(true)
    {
    }
};

struct HasMoreOwnersMoreBlockedDomains : HasMoreOwnersMoreDomains
{
    NewContact new_owner;
    bool remove_admin_c;

    HasMoreOwnersMoreBlockedDomains(LibFred::OperationContext& _ctx)
            : HasMoreOwnersMoreDomains(_ctx),
              new_owner(_ctx),
              remove_admin_c(true)
    {
    }
};

} // namespace Test::Backend::AdminBlock
} // namespace Test::Backend
} // namespace Test


#endif
