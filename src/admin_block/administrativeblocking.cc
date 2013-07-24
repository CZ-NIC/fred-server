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
#include "fredlib/domain/create_administrative_object_block_request_id.h"
#include "fredlib/domain/create_administrative_object_state_restore_request.h"
#include "fredlib/domain/create_administrative_object_state_restore_request_id.h"
#include "fredlib/domain/create_domain_name_blacklist.h"
#include "fredlib/domain/create_domain_name_blacklist_id.h"
#include "fredlib/domain/update_domain.h"
#include <memory>

namespace
{

enum { OBJECT_TYPE_DOMAIN = 3 };

}

namespace Registry
{
    namespace Administrative
    {

        StatusDescList* BlockingImpl::getBlockingStatusDescList(const std::string &_lang)
        {
            try {
                std::auto_ptr< StatusDescList > result(new StatusDescList);
                Fred::OperationContext ctx;
                Fred::GetBlockingStatusDescList blocking_status_desc_list;
                blocking_status_desc_list.set_lang(_lang);
                Fred::GetBlockingStatusDescList::StatusDescList &desc_list = blocking_status_desc_list.exec(ctx);
                result->length(desc_list.size());
                int n = 0;
                for (Fred::GetBlockingStatusDescList::StatusDescList::const_iterator pItem = desc_list.begin();
                     pItem != desc_list.end(); ++n, ++pItem) {
                    StatusDesc &item = (*result)[n];
                    item.id = pItem->state_id;
                    item.shortName = ::CORBA::string_dup(pItem->status.c_str());
                    item.name = ::CORBA::string_dup(pItem->desc.c_str());
                }
                return result.release();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        DomainOwnerChangeList* BlockingImpl::blockDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            const ::Registry::Administrative::StatusList &_status_list,
            ::Registry::Administrative::OwnerBlockMode _owner_block_mode,
            const std::string &_reason)
        {
            try {
                std::auto_ptr< DomainOwnerChangeList > result(new DomainOwnerChangeList);
                Fred::OperationContext ctx;
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
                return result.release();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        DomainIdHandleOwnerChangeList* BlockingImpl::blockDomainsId(
            const ::Registry::Administrative::DomainIdList &_domain_list,
            const ::Registry::Administrative::StatusList &_status_list,
            ::Registry::Administrative::OwnerBlockMode _owner_block_mode,
            const std::string &_reason)
        {
            try {
                std::auto_ptr< DomainIdHandleOwnerChangeList > result(new DomainIdHandleOwnerChangeList);
//                result->length(_domain_list.length());
                Fred::OperationContext ctx;
                Fred::StatusList status_list;
                for (unsigned idx = 0; idx < _status_list.length(); ++idx) {
                    status_list.push_back(_status_list[idx].in());
                }
                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
                    const Fred::ObjectId object_id = _domain_list[idx];
                    Fred::CreateAdministrativeObjectBlockRequestId create_object_block_request(object_id, status_list);
                    create_object_block_request.set_reason(_reason);
//                    ::Registry::Administrative::DomainIdHandleOwnerChange &result_item = (*result)[idx];
//                    result_item.domainId = object_id;
//                    result_item.domainHandle = ::CORBA::string_dup(create_object_block_request.exec(ctx).c_str());
                    // KEEP_OWNER / BLOCK_OWNER / BLOCK_OWNER_COPY
//                    if (_owner_block_mode == )
                    create_object_block_request.exec(ctx);
                    Fred::PerformObjectStateRequest(object_id).exec(ctx);
                }
                ctx.commit_transaction();
                return result.release();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        void BlockingImpl::restorePreAdministrativeBlockStates(
            const ::Registry::Administrative::DomainList &_domain_list,
            ::Registry::Administrative::NullableString *_new_owner,
            const std::string &_reason)
        {
            try {
                Fred::OperationContext ctx;
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

        void BlockingImpl::restorePreAdministrativeBlockStatesId(
            const ::Registry::Administrative::DomainIdList &_domain_list,
            ::Registry::Administrative::NullableString *_new_owner,
            const std::string &_reason)
        {
            try {
                Fred::OperationContext ctx;
                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
                    const Fred::ObjectId object_id = _domain_list[idx];
                    Fred::CreateAdministrativeObjectStateRestoreRequestId create_object_state_restore_request(object_id, _reason);
                    create_object_state_restore_request.exec(ctx);
                    Fred::PerformObjectStateRequest(object_id).exec(ctx);
                    if (_new_owner != NULL) {
                        Database::query_param_list param(object_id);
                        Database::Result registrar_fqdn_result = ctx.get_conn().exec_params(
                            "SELECT reg.handle,oreg.name "
                            "FROM object_registry oreg "
                            "JOIN registrar reg ON oreg.crid=reg.id "
                            "WHERE oreg.id=$1::bigint", param);
                        if (registrar_fqdn_result.size() <= 0) {
                            std::string errmsg("|| not found:object_id: ");
                            errmsg += boost::lexical_cast< std::string >(object_id);
                            errmsg += " |";
                            throw INTERNAL_SERVER_ERROR(errmsg.c_str());
                        }
                        const Database::Row &row = registrar_fqdn_result[0];
                        const std::string registrar = static_cast< std::string >(row[0]);
                        const std::string domain = static_cast< std::string >(row[1]);
                        Fred::UpdateDomain update_domain(domain, registrar);
//                        update_domain.add_admin_contact(*_new_owner);
                        update_domain.exec(ctx);
                    }
                }
                ctx.commit_transaction();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        void BlockingImpl::updateBlockDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            const ::Registry::Administrative::StatusList &_status_list,
            const std::string &_reason)
        {
            try {
                Fred::OperationContext ctx;
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

        void BlockingImpl::updateBlockDomainsId(
            const ::Registry::Administrative::DomainIdList &_domain_list,
            const ::Registry::Administrative::StatusList &_status_list,
            const std::string &_reason)
        {
            try {
                Fred::OperationContext ctx;
                Fred::StatusList status_list;
                for (unsigned idx = 0; idx < _status_list.length(); ++idx) {
                    status_list.push_back(_status_list[idx].in());
                }
                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
                    const Fred::ObjectId object_id = _domain_list[idx];
                    Fred::CreateAdministrativeObjectStateRestoreRequestId create_object_state_restore_request(object_id, _reason);
                    create_object_state_restore_request.exec(ctx);
                    Fred::PerformObjectStateRequest(object_id).exec(ctx);
                    Fred::CreateAdministrativeObjectBlockRequestId create_object_state_request(object_id, status_list);
                    create_object_state_request.set_reason(_reason);
                    create_object_state_request.exec(ctx);
                    Fred::PerformObjectStateRequest(object_id).exec(ctx);
                }
                ctx.commit_transaction();
            }
            catch (const std::exception &e) {
                throw INTERNAL_SERVER_ERROR(e.what());
            }
        }

        void BlockingImpl::unblockDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            ::Registry::Administrative::NullableString *_new_owner,
            bool _remove_admin_c,
            const std::string &_reason)
        {
        }

        void BlockingImpl::unblockDomainsId(
            const ::Registry::Administrative::DomainIdList &_domain_list,
            ::Registry::Administrative::NullableString *_new_owner,
            bool _remove_admin_c,
            const std::string &_reason)
        {
            this->restorePreAdministrativeBlockStatesId(_domain_list, _new_owner, _reason);
        }

        void BlockingImpl::blacklistAndDeleteDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            ::Registry::Administrative::NullableDate *_blacklist_to_date)
        {
        }

        void BlockingImpl::blacklistAndDeleteDomainsId(
            const ::Registry::Administrative::DomainIdList &_domain_list,
            ::Registry::Administrative::NullableDate *_blacklist_to_date)
        {
        }

        void BlockingImpl::blacklistDomains(
            const ::Registry::Administrative::DomainList &_domain_list,
            ::Registry::Administrative::NullableDate *_blacklist_to_date,
            bool _with_delete)
        {
            try {
                Fred::OperationContext ctx;
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

        void BlockingImpl::blacklistDomainsId(
            const ::Registry::Administrative::DomainIdList &_domain_list,
            ::Registry::Administrative::NullableDate *_blacklist_to_date,
            bool _with_delete)
        {
            try {
                Fred::OperationContext ctx;
                for (unsigned idx = 0; idx < _domain_list.length(); ++idx) {
                    const Fred::ObjectId object_id = _domain_list[idx];
                    Fred::CreateDomainNameBlacklistId create_domain_name_blacklist(object_id, "blacklistDomainsId() call");
                    create_domain_name_blacklist.exec(ctx);
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


        void BlockingImpl::unblacklistAndCreateDomainsId(
            const ::Registry::Administrative::DomainIdList &_domain_list,
            const std::string &_owner)
        {
        }

    }//namespace Administrative
}//namespace Registry



