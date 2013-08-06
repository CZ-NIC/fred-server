/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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

/**
 *  @server_i.cc
 *  implementational code for administrativeblocking IDL interface
 *  pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/corba ~/workspace/enum/idl/idl/AdministrativeBlocking.idl
 */


#include "server_i.h"
#include "corba_wrapper_decl.h"
#include "admin_block/administrativeblocking.h"
#include <string>


namespace Registry
{
    namespace Administrative
    {

        Server_i::Server_i(const std::string &_server_name)
        : bimpl_(new BlockingImpl(_server_name))
        {}

        Server_i::~Server_i()
        {}

        //   Methods corresponding to IDL attributes and operations
        StatusDescList* Server_i::getBlockingStatusDescList(const char* _lang)
        {
            return bimpl_->getBlockingStatusDescList(_lang);
        }

        DomainOwnerChangeList* Server_i::blockDomains(
            const ::Registry::Administrative::DomainList &domain_list,
            const ::Registry::Administrative::StatusList &status_list,
            ::Registry::Administrative::OwnerBlockMode owner_block_mode,
            const char *reason)
        {
            return bimpl_->blockDomains(domain_list, status_list, owner_block_mode, reason);
        }

        DomainIdHandleOwnerChangeList* Server_i::blockDomainsId(
            const ::Registry::Administrative::DomainIdList& domain_list,
            const ::Registry::Administrative::StatusList& status_list,
            ::Registry::Administrative::OwnerBlockMode owner_block_mode,
            ::Registry::Administrative::NullableDate *block_to_date,
            const char* reason,
            ::CORBA::ULongLong log_req_id)
        {
            return bimpl_->blockDomainsId(domain_list, status_list, owner_block_mode, reason, log_req_id);
        }

        void Server_i::updateBlockDomains(
            const ::Registry::Administrative::DomainList &domain_list,
            const ::Registry::Administrative::StatusList &status_list,
            const char *reason)
        {
            bimpl_->updateBlockDomains(domain_list, status_list, reason);
        }
        
        void Server_i::updateBlockDomainsId(
            const ::Registry::Administrative::DomainIdList &domain_list,
            const ::Registry::Administrative::StatusList &status_list,
            ::Registry::Administrative::NullableDate *block_to_date,
            const char *reason,
            ::CORBA::ULongLong log_req_id)
        {
            bimpl_->updateBlockDomainsId(domain_list, status_list, reason, log_req_id);
        }
        
        void Server_i::restorePreAdministrativeBlockStates(
            const ::Registry::Administrative::DomainList &domain_list,
            ::Registry::Administrative::NullableString *new_owner,
            const char *reason)
        {
            bimpl_->restorePreAdministrativeBlockStates(domain_list, new_owner, reason);
        }

        void Server_i::restorePreAdministrativeBlockStatesId(
            const ::Registry::Administrative::DomainIdList &domain_list,
            ::Registry::Administrative::NullableString *new_owner,
            const char* reason,
            ::CORBA::ULongLong log_req_id)
        {
            bimpl_->restorePreAdministrativeBlockStatesId(domain_list, new_owner, reason, log_req_id);
        }

        void Server_i::unblockDomains(
            const ::Registry::Administrative::DomainList &domain_list,
            ::Registry::Administrative::NullableString *new_owner,
            ::CORBA::Boolean remove_admin_c,
            const char* reason)
        {
            bimpl_->unblockDomains(domain_list, new_owner, remove_admin_c, reason);
        }
        
        void Server_i::unblockDomainsId(
            const ::Registry::Administrative::DomainIdList &domain_list,
            ::Registry::Administrative::NullableString *new_owner,
            ::CORBA::Boolean remove_admin_c,
            const char* reason,
            ::CORBA::ULongLong log_req_id)
        {
            bimpl_->unblockDomainsId(domain_list, new_owner, remove_admin_c, reason, log_req_id);
        }
        
        void Server_i::blacklistAndDeleteDomains(
            const ::Registry::Administrative::DomainList &domain_list,
            ::Registry::Administrative::NullableDate *blacklist_to_date)
        {
            bimpl_->blacklistAndDeleteDomains(domain_list, blacklist_to_date);
        }

        void Server_i::blacklistAndDeleteDomainsId(
            const ::Registry::Administrative::DomainIdList &domain_list,
            ::Registry::Administrative::NullableDate *blacklist_to_date,
            ::CORBA::ULongLong log_req_id)
        {
            bimpl_->blacklistAndDeleteDomainsId(domain_list, blacklist_to_date, log_req_id);
        }

        void Server_i::blacklistDomains(
            const ::Registry::Administrative::DomainList &domain_list,
            ::Registry::Administrative::NullableDate *blacklist_to_date,
            ::CORBA::Boolean with_delete)
        {
            bimpl_->blacklistDomains(domain_list, blacklist_to_date, with_delete);
        }

        void Server_i::blacklistDomainsId(
            const ::Registry::Administrative::DomainIdList &domain_list,
            ::Registry::Administrative::NullableDate *blacklist_to_date,
            ::CORBA::Boolean with_delete,
            ::CORBA::ULongLong log_req_id)
        {
            bimpl_->blacklistDomainsId(domain_list, blacklist_to_date, with_delete, log_req_id);
        }

        void Server_i::unblacklistAndCreateDomains(
            const ::Registry::Administrative::DomainList &domain_list,
            const char* owner)
        {
            bimpl_->unblacklistAndCreateDomains(domain_list, owner);
        }

//        void Server_i::unblacklistAndCreateDomainsId(
//            const ::Registry::Administrative::DomainIdList &domain_list,
//            const char *owner)
//        {
//            bimpl_->unblacklistAndCreateDomainsId(domain_list, owner);
//        }

    }//namespace Administrative
}//namespace Registry
