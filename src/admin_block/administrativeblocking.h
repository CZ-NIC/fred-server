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
 *  @administrativeblocking.h
 *  header of administrativeblocking implementation
 */

#ifndef ADMINISTRATIVEBLOCKING_H_
#define ADMINISTRATIVEBLOCKING_H_

#include <AdministrativeBlocking.hh>
#include <string>

namespace Registry
{
    namespace Administrative
    {

        class BlockingImpl
        {
        public:
            BlockingImpl(const std::string &_server_name):server_name_(_server_name) { }
            virtual ~BlockingImpl() { }

            StatusDescList* getBlockingStatusDescList(
                const std::string &_lang);

            DomainOwnerChangeList* blockDomains(
                const ::Registry::Administrative::DomainList &_domain_list,
                const ::Registry::Administrative::StatusList &_status_list,
                ::Registry::Administrative::OwnerBlockMode _owner_block_mode,
                const std::string &_reason);

            DomainIdHandleOwnerChangeList* blockDomainsId(
                const ::Registry::Administrative::DomainIdList &_domain_list,
                const ::Registry::Administrative::StatusList &_status_list,
                ::Registry::Administrative::OwnerBlockMode _owner_block_mode,
                const std::string &_reason,
                unsigned long long _log_req_id);

            void restorePreAdministrativeBlockStates(
                const ::Registry::Administrative::DomainList &_domain_list,
                ::Registry::Administrative::NullableString *_new_owner,
                const std::string &_reason);

            void restorePreAdministrativeBlockStatesId(
                const ::Registry::Administrative::DomainIdList &_domain_list,
                ::Registry::Administrative::NullableString *_new_owner,
                const std::string &_reason,
                unsigned long long _log_req_id);

            void updateBlockDomains(
                const ::Registry::Administrative::DomainList &_domain_list,
                const ::Registry::Administrative::StatusList &_status_list,
                const std::string &_reason);

            void updateBlockDomainsId(
                const ::Registry::Administrative::DomainIdList &_domain_list,
                const ::Registry::Administrative::StatusList &_status_list,
                const std::string &_reason,
                unsigned long long _log_req_id);

            void unblockDomains(
                const ::Registry::Administrative::DomainList &_domain_list,
                ::Registry::Administrative::NullableString *_new_owner,
                bool _remove_admin_c,
                const std::string &_reason);

            void unblockDomainsId(
                const ::Registry::Administrative::DomainIdList &_domain_list,
                ::Registry::Administrative::NullableString *_new_owner,
                bool _remove_admin_c,
                const std::string &_reason,
                unsigned long long _log_req_id);

            void blacklistAndDeleteDomains(
                const ::Registry::Administrative::DomainList &_domain_list,
                ::Registry::Administrative::NullableDate *_blacklist_to_date);

            void blacklistAndDeleteDomainsId(
                const ::Registry::Administrative::DomainIdList &_domain_list,
                ::Registry::Administrative::NullableDate *_blacklist_to_date,
                const std::string &_reason,
                unsigned long long _log_req_id);

            void blacklistDomains(
                const ::Registry::Administrative::DomainList &_domain_list,
                ::Registry::Administrative::NullableDate *_blacklist_to_date,
                bool _with_delete);

            void blacklistDomainsId(
                const ::Registry::Administrative::DomainIdList &_domain_list,
                ::Registry::Administrative::NullableDate *_blacklist_to_date,
                bool _with_delete,
                unsigned long long _log_req_id);

            void unblacklistAndCreateDomains(
                const ::Registry::Administrative::DomainList &_domain_list,
                const std::string &_owner);

//            void unblacklistAndCreateDomainsId(
//                const ::Registry::Administrative::DomainIdList &_domain_list,
//                const std::string &_owner);

        private:
            std::string server_name_;
        };//class BlockingImpl

    }//namespace Administrative
}//namespace Registry

#endif // ADMINISTRATIVEBLOCKING_H_
