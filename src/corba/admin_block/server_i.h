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
*  @server_i.h
*  header of administrativeblocking corba wrapper
*  pregenerated by $> omniidl -bcxx -Wba -Wbexample -C./src/corba ~/workspace/enum/idl/idl/AdministrativeBlocking.idl
*/
#ifndef SERVER_I_H_
#define SERVER_I_H_

#include <AdministrativeBlocking.hh>
#include <memory>
#include <string>

namespace Registry
{
    namespace Administrative
    {
        class BlockingImpl;//bimpl class

        ///administrativeblocking corba interface
        class Server_i: public POA_Registry::Administrative::Blocking
        {
        private:
            // do not copy
            const std::auto_ptr< BlockingImpl > bimpl_;
            Server_i(const Server_i&);//no body
            Server_i& operator= (const Server_i&);//no body

        public:
            // standard constructor
            Server_i(const std::string &_server_name);
            virtual ~Server_i();

            // methods corresponding to defined IDL attributes and operations
            virtual StatusDescList* getBlockingStatusDescList(const char* _lang);
            virtual DomainOwnerChangeList* blockDomains(const ::Registry::Administrative::DomainList& domain_list, const ::Registry::Administrative::StatusList& status_list, ::Registry::Administrative::OwnerBlockMode owner_block_mode, const char* reason);
            virtual void updateBlockDomains(const ::Registry::Administrative::DomainList& domain_list, const ::Registry::Administrative::StatusList& status_list, const char* reason);
            virtual void restorePreAdministrativeBlockStates(const ::Registry::Administrative::DomainList& domain_list, ::Registry::Administrative::NullableString* new_owner, const char* reason);
            virtual void unblockDomains(const ::Registry::Administrative::DomainList& domain_list, ::Registry::Administrative::NullableString* new_owner, ::CORBA::Boolean remove_admin_c, const char* reason);
            virtual void blacklistAndDeleteDomains(const ::Registry::Administrative::DomainList& domain_list, ::Registry::Administrative::NullableDate* blacklist_to_date);
            virtual void blacklistDomains(const ::Registry::Administrative::DomainList& domain_list, ::Registry::Administrative::NullableDate* blacklist_to_date, ::CORBA::Boolean with_delete);
            virtual void unblacklistAndCreateDomains(const ::Registry::Administrative::DomainList& domain_list, const char* owner);
            virtual DomainIdHandleOwnerChangeList* blockDomainsId(const ::Registry::Administrative::DomainIdList& domain_list, const ::Registry::Administrative::StatusList& status_list, ::Registry::Administrative::OwnerBlockMode owner_block_mode, ::Registry::Administrative::NullableDate *block_to_date, const char* reason, ::CORBA::ULongLong log_req_id);
            virtual void updateBlockDomainsId(const ::Registry::Administrative::DomainIdList& domain_list, const ::Registry::Administrative::StatusList& status_list, ::Registry::Administrative::NullableDate *block_to_date, const char* reason, ::CORBA::ULongLong log_req_id);
            virtual void restorePreAdministrativeBlockStatesId(const ::Registry::Administrative::DomainIdList& domain_list, ::Registry::Administrative::NullableString* new_owner, const char* reason, ::CORBA::ULongLong log_req_id);
            virtual void unblockDomainsId(const ::Registry::Administrative::DomainIdList& domain_list, ::Registry::Administrative::NullableString* new_owner, ::CORBA::Boolean remove_admin_c, const char* reason, ::CORBA::ULongLong log_req_id);
            virtual void blacklistAndDeleteDomainsId(const ::Registry::Administrative::DomainIdList& domain_list, ::Registry::Administrative::NullableDate* blacklist_to_date, ::CORBA::ULongLong log_req_id);
            virtual void blacklistDomainsId(const ::Registry::Administrative::DomainIdList& domain_list, ::Registry::Administrative::NullableDate* blacklist_to_date, ::CORBA::Boolean with_delete, ::CORBA::ULongLong log_req_id);
//            virtual void unblacklistAndCreateDomainsId(const ::Registry::Administrative::DomainIdList& domain_list, const char* owner, ::CORBA::ULongLong log_req_id);
        };//class Server_i
    }//namespace Administrative
}//namespace Registry

#endif //SERVER_I_H_
