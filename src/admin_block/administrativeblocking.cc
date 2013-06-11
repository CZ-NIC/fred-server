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
 *  @administrativeblocking.cc
 *  administrativeblocking implementation
 */

#include "administrativeblocking.h"
#include "corba/connection_releaser.h"
#include "fredlib/domain/get_blocking_status_desc_list.h"
#include "fredlib/domain/create_administrative_object_block_request.h"
#include "fredlib/domain/create_administrative_object_state_restore_request.h"
#include "fredlib/domain/create_domain_name_blacklist.h"
#include <memory>

namespace
{

enum { OBJECT_TYPE_DOMAIN = 3 };

}

namespace Registry
{
    namespace Administrative
    {
        std::string BlockingImpl::getName(
            const std::string &_lang)
        {
            if (_lang == "CZ") {
                return _lang;
            }
            throw INTERNAL_SERVER_ERROR(("bad language " + _lang).c_str());
        }//getName

        unsigned long long BlockingImpl::getLangID(
            const std::string &_lang)
        {
            if (_lang == "CZ") {
                return 0;
            }
            if (!_lang.empty()) {
                unsigned long long sum = 0;
                for (::size_t idx = 0; idx < _lang.length(); ++idx) {
                    sum += (unsigned long long)(_lang[idx]);
                }
                return sum;
            }
            throw INTERNAL_SERVER_ERROR("no language");
        }//getLangID

        StatusDescList* BlockingImpl::getBlockingStatusDescList(const std::string &_lang)
        {
            try {
                std::auto_ptr< StatusDescList > result(new StatusDescList);
                Fred::OperationContextTransaction ctx;
                Fred::GetBlockingStatusDescList blocking_status_desc_list;
                blocking_status_desc_list.set_lang(_lang);
                Fred::GetBlockingStatusDescList::StatusDescList &desc_list = blocking_status_desc_list.exec(ctx);
                result->length(desc_list.size());
                int n = 0;
                for (Fred::GetBlockingStatusDescList::StatusDescList::const_iterator pItem = desc_list.begin();
                     pItem != desc_list.end(); ++n, ++pItem) {
                    StatusDesc item;
                    item.id = pItem->state_id;
                    item.shortName = pItem->status.c_str();
                    item.name = pItem->desc.c_str();
                    (*result)[n] = item;
                }
                return result.release();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        void BlockingImpl::blockDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            const ::Registry::Administrative::StatusList &_status_list,
            ::CORBA::Boolean _block_owner,
            ::CORBA::Boolean _create_owner_copy)
        {
            try {
                Fred::OperationContextTransaction ctx;
                Fred::StatusList status_list;
                for (unsigned idx = 0; idx < _status_list.length(); ++idx) {
                    status_list.push_back(_status_list[idx].in());
                }
                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
                    const std::string domain = _domain_list[idx].in();
                    Fred::CreateAdministrativeObjectBlockRequest create_object_state_request(domain, OBJECT_TYPE_DOMAIN, status_list);
                    const Fred::ObjectId object_id = create_object_state_request.exec(ctx);
                    Fred::PerformObjectStateRequest(object_id).exec(ctx);
                }
                ctx.commit_transaction();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        void BlockingImpl::restorePreAdministrativeBlockStates(
            const ::Registry::Administrative::DomainList &_domain_list)
        {
            try {
                Fred::OperationContextTransaction ctx;
                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
                    const std::string domain = _domain_list[idx].in();
                    Fred::CreateAdministrativeObjectStateRestoreRequest create_object_state_restore_request(domain, OBJECT_TYPE_DOMAIN);
                    const Fred::ObjectId object_id = create_object_state_restore_request.exec(ctx);
                    Fred::PerformObjectStateRequest(object_id).exec(ctx);
                }
                ctx.commit_transaction();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }


        void BlockingImpl::updateBlockDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            const ::Registry::Administrative::StatusList &_status_list)
        {
            try {
                Fred::OperationContextTransaction ctx;
                Fred::StatusList status_list;
                for (unsigned idx = 0; idx < _status_list.length(); ++idx) {
                    status_list.push_back(_status_list[idx].in());
                }
//                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
//                    const std::string domain = _domain_list[idx].in();
//                    enum { OBJECT_TYPE_DOMAIN = 3 };
//                    Fred::CreateAdministrativeObjectBlockRequest create_object_state_request(domain, OBJECT_TYPE_DOMAIN, status_list);
//                    const Fred::ObjectId object_id = create_object_state_request.exec(ctx);
//                    Fred::PerformObjectStateRequest(object_id).exec(ctx);
//                }
                ctx.commit_transaction();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        void BlockingImpl::unblockDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            ::Registry::Administrative::NullableString *_new_owner,
            ::CORBA::Boolean _remove_admin_c)
        {
        }

        void BlockingImpl::blacklistAndDeleteDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            ::Registry::Administrative::NullableDate *_blacklist_to_date)
        {
        }

        void BlockingImpl::blacklistDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            ::Registry::Administrative::NullableDate *_blacklist_to_date,
            ::CORBA::Boolean _with_delete)
        {
            try {
                Fred::OperationContextTransaction ctx;
                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
                    const std::string domain = _domain_list[idx].in();
                    Fred::CreateDomainNameBlacklist(domain, "").exec(ctx);
                }
                ctx.commit_transaction();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        void BlockingImpl::unblacklistAndCreateDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            const std::string &_owner)
        {
        }

    }//namespace Administrative
}//namespace Registry



