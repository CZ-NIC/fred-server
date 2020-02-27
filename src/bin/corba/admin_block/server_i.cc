/*
 * Copyright (C) 2013-2020  CZ.NIC, z. s. p. o.
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
/**
 *  @server_i.cc
 *  implementational code for administrativeblocking IDL interface
 *  pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/bin/corba ~/workspace/enum/idl/idl/AdministrativeBlocking.idl
 */


#include "corba/NullableIsoDate.hh"

#include "src/backend/admin_block/administrativeblocking.hh"
#include "src/bin/corba/admin_block/corba_conversion.hh"
#include "src/bin/corba/admin_block/server_i.hh"
#include "src/bin/corba/util/corba_conversions_isodate.hh"
#include "src/bin/corba/util/corba_conversions_nullableisodate.hh"

#include <string>


namespace CorbaConversion {
namespace AdministrativeBlocking {

Server_i::Server_i(const std::string& _server_name)
    : bimpl_(new Fred::Backend::AdministrativeBlocking::BlockingImpl(_server_name))
{
}

Server_i::~Server_i()
{
}

//   Methods corresponding to IDL attributes and operations
Registry::Administrative::StatusDescList* Server_i::getBlockingStatusDescList(const char* _lang)
{
    try
    {
        return corba_wrap_status_desc_list(bimpl_->getBlockingStatusDescList(_lang));
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_INTERNAL_SERVER_ERROR& e)
    {
        throw corba_wrap_exception(e);
    }
}

Registry::Administrative::DomainOwnerChangeList* Server_i::blockDomains(
        const Registry::Administrative::DomainList& domain_list [[gnu::unused]],
        const Registry::Administrative::StatusList& status_list [[gnu::unused]],
        Registry::Administrative::OwnerBlockMode owner_block_mode [[gnu::unused]],
        const char* reason [[gnu::unused]])
{
    throw std::runtime_error("blockDomains not implemented");
}

Registry::Administrative::DomainIdHandleOwnerChangeList* Server_i::blockDomainsId(
        const Registry::Administrative::DomainIdList& domain_list,
        const Registry::Administrative::StatusList& status_list,
        Registry::Administrative::OwnerBlockMode owner_block_mode,
        Registry::NullableIsoDate* block_to_date,
        const char* reason,
        CORBA::ULongLong log_req_id)
{
    try
    {
        return corba_wrap_owner_change_list(bimpl_->blockDomainsId(corba_unwrap_domain_id_list(domain_list),
                corba_unwrap_status_list(status_list),
                corba_unwrap_owner_block_mode(owner_block_mode),
                Util::unwrap_NullableIsoDate_to_Nullable_boost_gregorian_date(block_to_date),
                reason,
                log_req_id));
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_NOT_FOUND& e)
    {
        throw corba_wrap_exception(e);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_UNKNOWN_STATUS& e)
    {
        throw corba_wrap_exception(e);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_ALREADY_BLOCKED& e)
    {
        throw corba_wrap_exception(e);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_OWNER_HAS_OTHER_DOMAIN& e)
    {
        throw corba_wrap_exception(e);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_INTERNAL_SERVER_ERROR& e)
    {
        throw corba_wrap_exception(e);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_CONTACT_BLOCK_PROHIBITED& e)
    {
        throw corba_wrap_exception(e);
    }
}

void Server_i::updateBlockDomains(
        const Registry::Administrative::DomainList& domain_list [[gnu::unused]],
        const Registry::Administrative::StatusList& status_list [[gnu::unused]],
        const char* reason [[gnu::unused]])
{
    throw std::runtime_error("updateBlockDomains not implemented");
}

void Server_i::updateBlockDomainsId(
        const Registry::Administrative::DomainIdList& domain_list,
        const Registry::Administrative::StatusList& status_list,
        Registry::NullableIsoDate* block_to_date,
        const char* reason,
        CORBA::ULongLong log_req_id)
{
    try
    {
        bimpl_->updateBlockDomainsId(corba_unwrap_domain_id_list(domain_list),
                corba_unwrap_status_list(status_list),
                Util::unwrap_NullableIsoDate_to_Nullable_boost_gregorian_date(block_to_date),
                reason,
                log_req_id);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_NOT_FOUND& e)
    {
        throw corba_wrap_exception(e);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_UNKNOWN_STATUS& e)
    {
        throw corba_wrap_exception(e);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_INTERNAL_SERVER_ERROR& e)
    {
        throw corba_wrap_exception(e);
    }
}

void Server_i::restorePreAdministrativeBlockStates(
        const Registry::Administrative::DomainList& domain_list [[gnu::unused]],
        Registry::Administrative::NullableString* new_owner [[gnu::unused]],
        const char* reason [[gnu::unused]])
{
    throw std::runtime_error("restorePreAdministrativeBlockStates not implemented");
}

void Server_i::restorePreAdministrativeBlockStatesId(
        const Registry::Administrative::DomainIdList& domain_list,
        Registry::Administrative::NullableString* new_owner,
        const char* reason,
        CORBA::ULongLong log_req_id)
{
    try
    {
        bimpl_->restorePreAdministrativeBlockStatesId(corba_unwrap_domain_id_list(domain_list),
                corba_unwrap_nullable_string(new_owner),
                reason,
                log_req_id);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_NOT_FOUND& e)
    {
        throw corba_wrap_exception(e);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_NOT_BLOCKED& e)
    {
        throw corba_wrap_exception(e);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_NEW_OWNER_DOES_NOT_EXISTS& e)
    {
        throw corba_wrap_exception(e);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_INTERNAL_SERVER_ERROR& e)
    {
        throw corba_wrap_exception(e);
    }
}

void Server_i::unblockDomains(
        const Registry::Administrative::DomainList& domain_list [[gnu::unused]],
        Registry::Administrative::NullableString* new_owner [[gnu::unused]],
        CORBA::Boolean remove_admin_c [[gnu::unused]],
        const char* reason [[gnu::unused]])
{
    throw std::runtime_error("unblockDomains not implemented");
}

void Server_i::unblockDomainsId(
        const Registry::Administrative::DomainIdList& domain_list,
        Registry::Administrative::NullableString* new_owner,
        CORBA::Boolean remove_admin_c,
        const char* reason,
        CORBA::ULongLong log_req_id)
{
    try
    {
        bimpl_->unblockDomainsId(corba_unwrap_domain_id_list(domain_list),
                corba_unwrap_nullable_string(new_owner),
                remove_admin_c,
                reason,
                log_req_id);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_NOT_FOUND& e)
    {
        throw corba_wrap_exception(e);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_NOT_BLOCKED& e)
    {
        throw corba_wrap_exception(e);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_NEW_OWNER_DOES_NOT_EXISTS& e)
    {
        throw corba_wrap_exception(e);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_INTERNAL_SERVER_ERROR& e)
    {
        throw corba_wrap_exception(e);
    }
}

void Server_i::blacklistAndDeleteDomains(
        const Registry::Administrative::DomainList& domain_list [[gnu::unused]],
        Registry::NullableIsoDate* blacklist_to_date [[gnu::unused]])
{
    throw std::runtime_error("blacklistAndDeleteDomains not implemented");
}

void Server_i::blacklistAndDeleteDomainsId(
        const Registry::Administrative::DomainIdList& domain_list,
        Registry::NullableIsoDate* blacklist_to_date,
        const char* reason,
        CORBA::ULongLong log_req_id)
{
    try
    {
        bimpl_->blacklistAndDeleteDomainsId(corba_unwrap_domain_id_list(domain_list),
                Util::unwrap_NullableIsoDate_to_Nullable_boost_gregorian_date(blacklist_to_date),
                reason,
                log_req_id);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_NOT_FOUND& e)
    {
        throw corba_wrap_exception(e);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_INTERNAL_SERVER_ERROR& e)
    {
        throw corba_wrap_exception(e);
    }
}

void Server_i::blacklistDomains(
        const Registry::Administrative::DomainList& domain_list [[gnu::unused]],
        Registry::NullableIsoDate* blacklist_to_date [[gnu::unused]],
        CORBA::Boolean with_delete [[gnu::unused]])
{
    throw std::runtime_error("blacklistDomains not implemented");
}

void Server_i::blacklistDomainsId(
        const Registry::Administrative::DomainIdList& domain_list,
        Registry::NullableIsoDate* blacklist_to_date,
        CORBA::Boolean with_delete,
        CORBA::ULongLong log_req_id)
{
    try
    {
        bimpl_->blacklistDomainsId(corba_unwrap_domain_id_list(domain_list),
                Util::unwrap_NullableIsoDate_to_Nullable_boost_gregorian_date(blacklist_to_date),
                with_delete,
                log_req_id);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_DOMAIN_ID_NOT_FOUND& e)
    {
        throw corba_wrap_exception(e);
    }
    catch (const Fred::Backend::AdministrativeBlocking::EX_INTERNAL_SERVER_ERROR& e)
    {
        throw corba_wrap_exception(e);
    }
}

void Server_i::unblacklistAndCreateDomains(
        const Registry::Administrative::DomainList& domain_list [[gnu::unused]],
        const char* owner [[gnu::unused]])
{
    throw std::runtime_error("unblacklistAndCreateDomains not implemented");
}

} //namespace AdministrativeBlocking
} // namespace CorbaConversion
