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
#include "fredlib/domain/get_blocking_status_desc_list.h"
#include "fredlib/domain/get_object_state_id_map.h"
#include "fredlib/domain/create_administrative_object_block_request.h"
#include "fredlib/domain/create_administrative_object_block_request_id.h"
#include "fredlib/domain/create_administrative_object_state_restore_request.h"
#include "fredlib/domain/create_administrative_object_state_restore_request_id.h"
#include "fredlib/domain/create_domain_name_blacklist.h"
#include "fredlib/domain/create_domain_name_blacklist_id.h"
#include "fredlib/domain/clear_administrative_object_state_request_id.h"
#include "fredlib/domain/update_domain.h"
#include "fredlib/domain/delete_domain.h"
#include "fredlib/domain/info_domain.h"
#include "fredlib/domain/copy_contact.h"
#include <memory>
#include <map>

namespace
{

enum { OBJECT_TYPE_DOMAIN = 3 };

}

namespace Registry
{
    namespace Administrative
    {

        Fred::GetBlockingStatusDescList::StatusDescList BlockingImpl::getBlockingStatusDescList(const std::string &_lang)
        {
            try {
                Fred::OperationContext ctx;
                Fred::GetBlockingStatusDescList blocking_status_desc_list;
                blocking_status_desc_list.set_lang(_lang);
                Fred::GetBlockingStatusDescList::StatusDescList &desc_list = blocking_status_desc_list.exec(ctx);
                ctx.commit_transaction();
                return desc_list;
            }
            catch (const std::exception &e) {
                EX_INTERNAL_SERVER_ERROR ex;
                ex.what = e.what();
                throw ex;
            }
        }

        namespace
        {
            struct OwnerCopy
            {
                std::string old_owner_handle;
                Fred::ObjectId new_owner_id;
                std::string new_owner_handle;
            };

            typedef std::map< Fred::ObjectId, Fred::ObjectId > DomainIdOwnerId;
            typedef std::map< Fred::ObjectId, OwnerCopy > OwnerIdOwnerCopy;
            typedef std::map< Fred::ObjectId, std::string > DomainIdHandle;

            static const std::string owner_copy_suffix = "-ABC_"; //AdministrativeBlockingCopy
            static const std::string owner_copy_zero_idx = "00000";

            std::string get_copy_owner_handle(
                const std::string &_owner_handle,
                unsigned long long _log_req_id,
                Fred::OperationContext &_ctx)
            {
                std::string new_owner_handle = _owner_handle + owner_copy_suffix;
                Database::Result last_owner_res = _ctx.get_conn().exec_params(
                    "SELECT name "
                    "FROM object_registry "
                    "WHERE (type=$1::bigint) AND "
                          "($2::text<name) AND "
                          "(name LIKE $3::text) AND "
                          "erdate IS NULL "
                    "ORDER BY name DESC LIMIT 1", Database::query_param_list(Fred::CopyContact::OBJECT_TYPE_ID_CONTACT)
                                                                            (new_owner_handle)
                                                                            (new_owner_handle + "%"));
                if (last_owner_res.size() == 0) {
                    new_owner_handle += owner_copy_zero_idx;
                    return new_owner_handle;
                }
                const std::string last_owner_name = static_cast< std::string >(last_owner_res[0][0]);
                const std::string str_copy_idx = last_owner_name.substr(new_owner_handle.length());
                ::size_t copy_idx = 0;
                for (const char *pC = str_copy_idx.c_str(); *pC != '\0'; ++pC) {
                    const char c = *pC;
                    if ((c < '0') || ('9' < c)) {
                        new_owner_handle = last_owner_name + owner_copy_suffix + owner_copy_zero_idx;
                        return new_owner_handle;
                    }
                    copy_idx = 10 * copy_idx + int(c) - int('0');
                }
                ++copy_idx;
                std::ostringstream idx;
                idx << std::setw(owner_copy_zero_idx.length()) << std::setfill('0') << copy_idx;
                new_owner_handle += idx.str();
                return new_owner_handle;
            }

            std::string get_sys_registrar(Fred::OperationContext &_ctx, bool &_is_system)
            {
                Database::Result sys_registrar_res = _ctx.get_conn().exec(
                    "SELECT handle,system "
                    "FROM registrar ORDER BY system DESC,id LIMIT 1");
                if (sys_registrar_res.size() == 1) {
                    _is_system = static_cast< bool >(sys_registrar_res[0][1]);
                    return static_cast< std::string >(sys_registrar_res[0][0]);
                }
                if (sys_registrar_res.size() == 0) {
                    throw std::runtime_error("no registrar found");
                }
                throw std::runtime_error("SELECT with LIMIT 1 return more then 1 row");
            }

            std::string get_object_handle(Fred::OperationContext &_ctx, Fred::ObjectId _object_id, bool _lock = true);
            
            std::string get_object_handle(Fred::OperationContext &_ctx, Fred::ObjectId _object_id, bool _lock)
            {
                const std::string query = _lock ?
                                              "SELECT name "
                                              "FROM object_registry "
                                              "WHERE id=$1::integer " 
                                              "FOR UPDATE"
                                          :
                                              "SELECT name "
                                              "FROM object_registry "
                                              "WHERE id=$1::integer";
                Database::Result handle_res = _ctx.get_conn().exec_params(query, Database::query_param_list(_object_id));
                if (handle_res.size() == 1) {
                    return static_cast< std::string >(handle_res[0][0]);
                }
                if (handle_res.size() == 0) {
                    throw std::runtime_error("object not found");
                }
                throw std::runtime_error("too many objects");
            }

            void copy_domain_owners(
                const std::string &_sys_registrar_handle,
                const IdlDomainIdList &_domain_list,
                DomainIdOwnerId &_domain_id_owner_id,
                OwnerIdOwnerCopy &_owner_id_owner_copy,
                unsigned long long _log_req_id,
                Fred::OperationContext &_ctx)
            {
                std::ostringstream query;
                for (IdlDomainIdList::const_iterator pObjectId = _domain_list.begin(); pObjectId != _domain_list.end(); ++pObjectId) {
                    if (query.str().empty()) {
                        query << "SELECT d.id,d.registrant,obr.name "
                                 "FROM domain d "
                                 "JOIN object_registry obr ON obr.id=d.registrant "
                                 "WHERE d.id IN (";
                    }
                    else {
                        query << ",";
                    }
                    query << (*pObjectId);
                }
                query << ") FOR UPDATE OF obr";
                Database::Result domain_info_res = _ctx.get_conn().exec(query.str());
                for (unsigned idx = 0; idx < domain_info_res.size(); ++idx) {
                    OwnerCopy item;
                    const Fred::ObjectId old_owner_id = static_cast< Fred::ObjectId >(domain_info_res[idx][1]);
                    if (_owner_id_owner_copy.find(old_owner_id) == _owner_id_owner_copy.end()) {
                        _owner_id_owner_copy[old_owner_id].old_owner_handle = static_cast< std::string >(domain_info_res[idx][2]);
                    }
                    const Fred::ObjectId domain_id = static_cast< Fred::ObjectId >(domain_info_res[idx][0]);
                    _domain_id_owner_id[domain_id] = old_owner_id;
                }
                for (OwnerIdOwnerCopy::iterator pOwnerCopy = _owner_id_owner_copy.begin(); pOwnerCopy != _owner_id_owner_copy.end(); ++pOwnerCopy) {
                    pOwnerCopy->second.new_owner_handle = get_copy_owner_handle(pOwnerCopy->second.old_owner_handle, _log_req_id, _ctx);
                    Fred::CopyContact copy_contact(pOwnerCopy->second.old_owner_handle,
                                                   pOwnerCopy->second.new_owner_handle,
                                                   _sys_registrar_handle,
                                                   _log_req_id);
                    pOwnerCopy->second.new_owner_id = copy_contact.exec(_ctx);
                }
            }

            void check_owner_has_no_other_domains(
                const IdlDomainIdList &_domain_list,
                Fred::OperationContext &_ctx)
            {
                if (_domain_list.empty()) {
                    return;
                }
                IdlDomainIdList::const_iterator pDomainId = _domain_list.begin();
                Database::query_param_list param(*pDomainId);
                std::ostringstream set_of_domain_id;
                set_of_domain_id << "($" << param.size() << "::bigint";
                for (++pDomainId; pDomainId != _domain_list.end(); ++pDomainId) {
                    param(*pDomainId);
                    set_of_domain_id << ",$" << param.size() << "::bigint";
                }
                set_of_domain_id << ")";
                Database::Result check_res = _ctx.get_conn().exec_params(
                    "SELECT d.id,dh.name,d.registrant,oh.name "
                    "FROM domain d "
                    "JOIN object_registry dh ON dh.id=d.id "
                    "JOIN object_registry oh ON oh.id=d.registrant "
                    "WHERE d.registrant IN (SELECT registrant "
                                           "FROM domain "
                                           "WHERE id IN " + set_of_domain_id.str() + " GROUP BY registrant) AND "
                          "d.id NOT IN " + set_of_domain_id.str(), param);
                if (check_res.size() <= 0) {
                    return;
                }
                EX_OWNER_HAS_OTHER_DOMAIN ex;
                for (unsigned idx = 0; idx < check_res.size(); ++idx) {
                    EX_DOMAIN_ID_ALREADY_BLOCKED::Item domain;
                    domain.domain_id = static_cast< Fred::ObjectId >(check_res[idx][0]);
                    domain.domain_handle = static_cast< std::string >(check_res[idx][1]);
                    const Fred::ObjectId owner_id = static_cast< Fred::ObjectId >(check_res[idx][2]);
                    EX_OWNER_HAS_OTHER_DOMAIN::Type::iterator pItem = ex.what.find(owner_id);
                    if (pItem == ex.what.end()) {
                        EX_OWNER_HAS_OTHER_DOMAIN::Item item;
                        item.owner_handle = static_cast< std::string >(check_res[idx][3]);
                        pItem = ex.what.insert(std::make_pair(owner_id, item)).first;
                    }
                    pItem->second.domain.insert(domain);
                }
                throw ex;
            }

            void check_owner_is_blockable(
                const IdlDomainIdList &_domain_list,
                Fred::OperationContext &_ctx)
            {
                if (_domain_list.empty()) {
                    return;
                }
                IdlDomainIdList::const_iterator pDomainId = _domain_list.begin();
                Database::query_param_list param(*pDomainId);
                std::ostringstream set_of_domain_id;
                set_of_domain_id << "($" << param.size() << "::bigint";
                for (++pDomainId; pDomainId != _domain_list.end(); ++pDomainId) {
                    param(*pDomainId);
                    set_of_domain_id << ",$" << param.size() << "::bigint";
                }
                set_of_domain_id << ")";
                Database::Result check_res = _ctx.get_conn().exec_params(
                    "SELECT obr.id,obr.name "
                    "FROM domain d "
                    "JOIN object_registry obr ON (obr.id=d.registrant AND obr.type=1) "
                    "JOIN object_state os ON os.object_id=d.registrant "
                    "JOIN enum_object_states eos ON (eos.id=os.state_id AND eos.name='mojeidContact') "
                    "WHERE obr.erdate IS NULL AND "
                          "os.valid_from<=CURRENT_TIMESTAMP AND (CURRENT_TIMESTAMP<os.valid_to OR os.valid_to IS NULL) AND "
                          "d.id IN " + set_of_domain_id.str() + " "
                    "GROUP BY obr.id,obr.name", param);
                if (check_res.size() <= 0) {
                    return;
                }
                EX_CONTACT_BLOCK_PROHIBITED ex;
                for (unsigned idx = 0; idx < check_res.size(); ++idx) {
                    EX_CONTACT_BLOCK_PROHIBITED::Item contact;
                    contact.contact_id = static_cast< Fred::ObjectId >(check_res[idx][0]);
                    contact.contact_handle = static_cast< std::string >(check_res[idx][1]);
                    ex.what.insert(contact);
                }
                throw ex;
            }

            DomainIdHandle& get_domain_handle(
                const IdlDomainIdList &_domain_list,
                DomainIdHandle &_result,
                Fred::OperationContext &_ctx)
            {
                _result.clear();
                if (_domain_list.empty()) {
                    return _result;
                }
                std::ostringstream query;
                IdlDomainIdList::const_iterator pDomainId = _domain_list.begin();
                Database::query_param_list param(*pDomainId);
                query << "SELECT obr.id,obr.name "
                         "FROM object_registry obr "
                         "JOIN domain d ON d.id=obr.id "
                         "WHERE obr.id IN ($" << param.size() << "::bigint";
                for (++pDomainId; pDomainId != _domain_list.end(); ++pDomainId) {
                    param(*pDomainId);
                    query << ",$" << param.size() << "::bigint";
                }
                query << ") AND obr.erdate IS NULL "
                         "FOR UPDATE OF obr";
                Database::Result domain_id_handle_res = _ctx.get_conn().exec_params(query.str(), param);
                for (::size_t idx = 0; idx < domain_id_handle_res.size(); ++idx) {
                    const Database::Row &row = domain_id_handle_res[idx];
                    _result[static_cast< Fred::ObjectId >(row[0])] = static_cast< std::string >(row[1]);
                }
                if (_domain_list.size() == _result.size()) {
                    return _result;
                }
                EX_DOMAIN_ID_NOT_FOUND e;
                for (IdlDomainIdList::const_iterator pDomainId = _domain_list.begin();
                     pDomainId != _domain_list.end(); ++pDomainId) {
                    if (_result.find(*pDomainId) == _result.end()) {
                        e.what.insert(*pDomainId);
                    }
                }
                throw e;
            }

        }

        IdlOwnerChangeList BlockingImpl::blockDomainsId(
            const IdlDomainIdList &_domain_list,
            const Fred::StatusList &_status_list,
            IdlOwnerBlockMode _owner_block_mode,
            const Nullable< boost::gregorian::date > &_block_to_date,
            const std::string &_reason,
            unsigned long long _log_req_id)
        {
            EX_DOMAIN_ID_NOT_FOUND domain_id_not_found;
            EX_UNKNOWN_STATUS unknown_status;
            EX_DOMAIN_ID_ALREADY_BLOCKED domain_id_already_blocked;
            EX_CONTACT_BLOCK_PROHIBITED contact_block_prohibited;
            try {
                IdlOwnerChangeList result;
                Fred::OperationContext ctx;
                DomainIdHandle domain_id_handle;
                get_domain_handle(_domain_list, domain_id_handle, ctx);
                Fred::StatusList contact_status_list;
                DomainIdOwnerId domain_id_owner_id;
                OwnerIdOwnerCopy owner_id_owner_copy;
                bool is_sys_registrar;
                std::string sys_registrar;
                if ((_owner_block_mode == OWNER_BLOCK_MODE_BLOCK_OWNER) ||
                    (_owner_block_mode == OWNER_BLOCK_MODE_BLOCK_OWNER_COPY)) {
                    Fred::GetObjectStateIdMap::StateIdMap state_id;
                    Fred::GetObjectStateIdMap::get_result(ctx, _status_list, Fred::CopyContact::OBJECT_TYPE_ID_CONTACT, state_id);
                    for (Fred::GetObjectStateIdMap::StateIdMap::const_iterator pState = state_id.begin();
                         pState != state_id.end(); ++pState) {
                        contact_status_list.insert(pState->first);
                    }
                    if (_owner_block_mode == OWNER_BLOCK_MODE_BLOCK_OWNER_COPY) {
                        if (sys_registrar.empty()) {
                            sys_registrar = get_sys_registrar(ctx, is_sys_registrar);
                        }
                        if (!is_sys_registrar) {
                            EX_INTERNAL_SERVER_ERROR e;
                            e.what = "system registrar not found";
                            throw e;
                        }
                        copy_domain_owners(sys_registrar, _domain_list, domain_id_owner_id, owner_id_owner_copy, _log_req_id, ctx);
                    }
                    else if (_owner_block_mode == OWNER_BLOCK_MODE_BLOCK_OWNER) {
                        check_owner_has_no_other_domains(_domain_list, ctx);
                        check_owner_is_blockable(_domain_list, ctx);
                    }
                }
                StringSet contact_blocked;
                boost::posix_time::ptime block_time_limit;
                if (!_block_to_date.isnull()) {
                    // block to date 12:00:00 https://admin.nic.cz/ticket/6314#comment:50
                    block_time_limit = boost::posix_time::ptime(static_cast< boost::gregorian::date >(_block_to_date),
                                                                boost::posix_time::time_duration(12, 0, 0));
                }
                for (IdlDomainIdList::const_iterator pObjectId = _domain_list.begin(); pObjectId != _domain_list.end(); ++pObjectId) {
                    try {
                        const Fred::ObjectId object_id = *pObjectId;
                        Fred::CreateAdministrativeObjectBlockRequestId create_object_block_request(object_id, _status_list);
                        create_object_block_request.set_reason(_reason);
                        if (!_block_to_date.isnull()) {
                            create_object_block_request.set_valid_to(block_time_limit);
                        }
                        if (_owner_block_mode == OWNER_BLOCK_MODE_BLOCK_OWNER) {
                            Database::query_param_list param(object_id);
                            Database::Result registrant_result = ctx.get_conn().exec_params(
                                "SELECT rc.id,rc.name "
                                "FROM domain d "
                                "JOIN object_registry rc ON rc.id=d.registrant "
                                "WHERE d.id=$1::bigint "
                                "FOR UPDATE OF rc", param);
                            if (registrant_result.size() <= 0) {
                                domain_id_not_found.what.insert(object_id);
                                continue;
                            }
                            const Database::Row &row = registrant_result[0];
                            const Fred::ObjectId registrant_id = static_cast< Fred::ObjectId >(row[0]);
                            const std::string registrant = static_cast< std::string >(row[1]);
                            if ((contact_blocked.find(registrant) == contact_blocked.end()) && !contact_status_list.empty()) {
                                contact_blocked.insert(registrant);
                                Fred::CreateAdministrativeObjectBlockRequestId block_owner_request(registrant_id, contact_status_list);
                                block_owner_request.set_reason(_reason);
                                if (!_block_to_date.isnull()) {
                                    block_owner_request.set_valid_to(block_time_limit);
                                }
                                block_owner_request.exec(ctx);
                                Fred::PerformObjectStateRequest(registrant_id).exec(ctx);
                            }
                            create_object_block_request.exec(ctx);
                        }
                        else if (_owner_block_mode == OWNER_BLOCK_MODE_BLOCK_OWNER_COPY) {
                            const std::string domain = create_object_block_request.exec(ctx);
                            IdlOwnerChange result_item;
                            result_item.domain_id = object_id;
                            result_item.domain_handle = domain;
                            result_item.old_owner_id = domain_id_owner_id[object_id];
                            const OwnerCopy &owner_copy = owner_id_owner_copy[result_item.old_owner_id];
                            result_item.old_owner_handle = owner_copy.old_owner_handle;
                            result_item.new_owner_handle = owner_copy.new_owner_handle;
                            result_item.new_owner_id = owner_copy.new_owner_id;
                            if (!contact_status_list.empty()) {
                                if (sys_registrar.empty()) {
                                    sys_registrar = get_sys_registrar(ctx, is_sys_registrar);
                                }
                                if (!is_sys_registrar) {
                                    EX_INTERNAL_SERVER_ERROR e;
                                    e.what = "system registrar not found";
                                    throw e;
                                }
                                Fred::UpdateDomain update_domain(domain, sys_registrar);
                                update_domain.set_registrant(result_item.new_owner_handle);
                                if (0 < _log_req_id) {
                                    update_domain.set_logd_request_id(_log_req_id);
                                }
                                update_domain.exec(ctx);
                            }
                            result.push_back(result_item);
                        }
                        else { // KEEP_OWNER 
                            create_object_block_request.exec(ctx);
                        }
                        Fred::PerformObjectStateRequest(object_id).exec(ctx);
                    }
                    catch (const Fred::CreateObjectStateRequestId::Exception &e) {
                        if (e.is_set_object_id_not_found()) {
                            domain_id_not_found.what.insert(e.get_object_id_not_found());
                        }
                        else if (e.is_set_vector_of_state_not_found()) {
                            std::vector< std::string > state_not_found = e.get_vector_of_state_not_found();
                            for (std::vector< std::string >::const_iterator pStat = state_not_found.begin();
                                 pStat != state_not_found.end(); ++pStat) {
                                unknown_status.what.insert(*pStat);
                            }
                        }
                        else {
                            throw std::runtime_error("Fred::CreateObjectStateRequestId::Exception");
                        }
                    }
                    catch (const Fred::CreateAdministrativeObjectBlockRequestId::Exception &e) {
                        if (e.is_set_server_blocked_present()) {
                            EX_DOMAIN_ID_ALREADY_BLOCKED::Item e_item;
                            e_item.domain_id = e.get_server_blocked_present();
                            e_item.domain_handle = domain_id_handle[e_item.domain_id];
                            domain_id_already_blocked.what.insert(e_item);
                        }
                        else if (e.is_set_vector_of_state_not_found()) {
                            std::vector< std::string > state_not_found = e.get_vector_of_state_not_found();
                            for (std::vector< std::string >::const_iterator pStat = state_not_found.begin();
                                 pStat != state_not_found.end(); ++pStat) {
                                unknown_status.what.insert(*pStat);
                            }
                        }
                        else {
                            throw std::runtime_error("Fred::CreateAdministrativeObjectBlockRequestId::Exception");
                        }
                    }
                }
                if (_owner_block_mode == OWNER_BLOCK_MODE_BLOCK_OWNER_COPY) {
                    for (OwnerIdOwnerCopy::const_iterator pOwnerCopy = owner_id_owner_copy.begin();
                         pOwnerCopy != owner_id_owner_copy.end(); ++pOwnerCopy) {
                        Fred::CreateAdministrativeObjectBlockRequestId block_owner_request(pOwnerCopy->second.new_owner_id,
                                                                                           contact_status_list);
                        block_owner_request.set_reason(_reason);
                        if (!_block_to_date.isnull()) {
                            block_owner_request.set_valid_to(block_time_limit);
                        }
                        block_owner_request.exec(ctx);
                        Fred::PerformObjectStateRequest(pOwnerCopy->second.new_owner_id).exec(ctx);
                    }
                }
                if (!domain_id_not_found.what.empty()) {
                    throw domain_id_not_found;
                }
                if (!domain_id_already_blocked.what.empty()) {
                    throw domain_id_already_blocked;
                }
                if (!unknown_status.what.empty()) {
                    throw unknown_status;
                }
                ctx.commit_transaction();
                return result;
            }
            catch (const EX_DOMAIN_ID_NOT_FOUND&) {
                throw;
            }
            catch (const EX_UNKNOWN_STATUS&) {
                throw;
            }
            catch (const EX_DOMAIN_ID_ALREADY_BLOCKED&) {
                throw;
            }
            catch (const EX_OWNER_HAS_OTHER_DOMAIN&) {
                throw;
            }
            catch (const EX_CONTACT_BLOCK_PROHIBITED&) {
                throw;
            }
            catch (const std::exception &e) {
                EX_INTERNAL_SERVER_ERROR ex;
                ex.what = e.what();
                throw ex;
            }
        }

        void BlockingImpl::restorePreAdministrativeBlockStatesId(
            const IdlDomainIdList &_domain_list,
            const Nullable< std::string > &_new_owner,
            const std::string &_reason,
            unsigned long long _log_req_id)
        {
            EX_DOMAIN_ID_NOT_BLOCKED domain_id_not_blocked;
            try {
                Fred::OperationContext ctx;
                DomainIdHandle domain_id_handle;
                get_domain_handle(_domain_list, domain_id_handle, ctx);
                bool is_sys_registrar;
                std::string sys_registrar;
                for (IdlDomainIdList::const_iterator pDomainId = _domain_list.begin(); pDomainId != _domain_list.end(); ++pDomainId) {
                    const Fred::ObjectId object_id = *pDomainId;
                    try {
                        Fred::CreateAdministrativeObjectStateRestoreRequestId create_object_state_restore_request(object_id, _reason, _log_req_id);
                        create_object_state_restore_request.exec(ctx);
                        Fred::PerformObjectStateRequest(object_id).exec(ctx);
                        const std::string fqdn = get_object_handle(ctx, object_id);
                        if (sys_registrar.empty()) {
                            sys_registrar = get_sys_registrar(ctx, is_sys_registrar);
                        }
                        const boost::gregorian::date expiration_date = Fred::InfoDomain(fqdn, sys_registrar).exec(ctx).info_domain_data.expiration_date;
                        const boost::gregorian::date today(boost::gregorian::day_clock::universal_day());
                        const bool set_expire_today = expiration_date < today;
                        const bool new_owner_is_set = !(_new_owner.isnull() || static_cast< std::string >(_new_owner).empty());
                        if (new_owner_is_set || set_expire_today) {
                            if (sys_registrar.empty()) {
                                sys_registrar = get_sys_registrar(ctx, is_sys_registrar);
                            }
                            if (!is_sys_registrar) {
                                EX_INTERNAL_SERVER_ERROR e;
                                e.what = "system registrar not found";
                                throw e;
                            }
                            Fred::UpdateDomain update_domain(fqdn, sys_registrar);
                            if (new_owner_is_set) {
                                update_domain.set_registrant(_new_owner);
                            }
                            if (set_expire_today) {
                                update_domain.set_domain_expiration(today);
                            }
                            if (0 < _log_req_id) {
                                update_domain.set_logd_request_id(_log_req_id);
                            }
                            update_domain.exec(ctx);
                        }
                    }
                    catch (const Fred::CreateAdministrativeObjectStateRestoreRequestId::Exception &e) {
                        if (e.is_set_server_blocked_absent()) {
                            EX_DOMAIN_ID_NOT_BLOCKED::Item e_item;
                            e_item.domain_id = e.get_server_blocked_absent();
                            e_item.domain_handle = domain_id_handle[e_item.domain_id];
                            domain_id_not_blocked.what.insert(e_item);
                        }
                        else {
                            throw;
                        }
                    }
                    catch (const Fred::UpdateDomain::Exception &e) {
                        if (e.is_set_unknown_registrant_handle()) {
                            EX_NEW_OWNER_DOES_NOT_EXISTS ex;
                            ex.what = e.get_unknown_registrant_handle();
                            throw ex;
                        }
                        else {
                            throw std::runtime_error("Fred::UpdateDomain::Exception");
                        }
                    }
                }
                if (!domain_id_not_blocked.what.empty()) {
                    throw domain_id_not_blocked;
                }
                ctx.commit_transaction();
            }
            catch (const EX_DOMAIN_ID_NOT_FOUND&) {
                throw;
            }
            catch (const EX_DOMAIN_ID_NOT_BLOCKED&) {
                throw;
            }
            catch (const EX_NEW_OWNER_DOES_NOT_EXISTS&) {
                throw;
            }
            catch (const std::exception &e) {
                EX_INTERNAL_SERVER_ERROR ex;
                ex.what = e.what();
                throw ex;
            }
        }

        void BlockingImpl::updateBlockDomainsId(
            const IdlDomainIdList &_domain_list,
            const Fred::StatusList &_status_list,
            const Nullable< boost::gregorian::date > &_block_to_date,
            const std::string &_reason,
            unsigned long long _log_req_id)
        {
            EX_DOMAIN_ID_NOT_FOUND domain_id_not_found;
            EX_UNKNOWN_STATUS unknown_status;
            try {
                boost::posix_time::ptime block_time_limit;
                if (!_block_to_date.isnull()) {
                    // block to date 12:00:00 https://admin.nic.cz/ticket/6314#comment:50
                    block_time_limit = boost::posix_time::ptime(static_cast< boost::gregorian::date >(_block_to_date),
                                                                boost::posix_time::time_duration(12, 0, 0));
                }
                Fred::OperationContext ctx;
                for (IdlDomainIdList::const_iterator pDomainId = _domain_list.begin(); pDomainId != _domain_list.end(); ++pDomainId) {
                    const Fred::ObjectId object_id = *pDomainId;
                    try {
                        Fred::CreateAdministrativeObjectStateRestoreRequestId create_object_state_restore_request(object_id, _reason, _log_req_id);
                        create_object_state_restore_request.exec(ctx);
                        Fred::PerformObjectStateRequest(object_id).exec(ctx);
                        Fred::CreateAdministrativeObjectBlockRequestId create_object_state_request(object_id, _status_list);
                        if (!_block_to_date.isnull()) {
                            create_object_state_request.set_valid_to(block_time_limit);
                        }
                        create_object_state_request.set_reason(_reason);
                        create_object_state_request.exec(ctx);
                        Fred::PerformObjectStateRequest(object_id).exec(ctx);
                    }
                    catch (const Fred::CreateObjectStateRequestId::Exception &e) {
                        if (e.is_set_object_id_not_found()) {
                            domain_id_not_found.what.insert(e.get_object_id_not_found());
                        }
                        else if (e.is_set_vector_of_state_not_found()) {
                            std::vector< std::string > state_not_found = e.get_vector_of_state_not_found();
                            for (std::vector< std::string >::const_iterator pStat = state_not_found.begin();
                                 pStat != state_not_found.end(); ++pStat) {
                                unknown_status.what.insert(*pStat);
                            }
                        }
                        else {
                            throw std::runtime_error("Fred::CreateObjectStateRequestId::Exception");
                        }
                    }
                    catch (const Fred::CreateAdministrativeObjectBlockRequestId::Exception &e) {
                        if (e.is_set_vector_of_state_not_found()) {
                            std::vector< std::string > state_not_found = e.get_vector_of_state_not_found();
                            for (std::vector< std::string >::const_iterator pStat = state_not_found.begin();
                                 pStat != state_not_found.end(); ++pStat) {
                                unknown_status.what.insert(*pStat);
                            }
                        }
                        else {
                            throw std::runtime_error("Fred::CreateAdministrativeObjectBlockRequestId::Exception");
                        }
                    }
                }
                if (!domain_id_not_found.what.empty()) {
                    throw domain_id_not_found;
                }
                if (!unknown_status.what.empty()) {
                    throw unknown_status;
                }
                ctx.commit_transaction();
            }
            catch (const EX_DOMAIN_ID_NOT_FOUND&) {
                throw;
            }
            catch (const EX_UNKNOWN_STATUS&) {
                throw;
            }
            catch (const std::exception &e) {
                EX_INTERNAL_SERVER_ERROR ex;
                ex.what = e.what();
                throw ex;
            }
        }

        void BlockingImpl::unblockDomainsId(
            const IdlDomainIdList &_domain_list,
            const Nullable< std::string > &_new_owner,
            bool _remove_admin_c,
            const std::string &_reason,
            unsigned long long _log_req_id)
        {
            EX_DOMAIN_ID_NOT_BLOCKED domain_id_not_blocked;
            try {
                Fred::OperationContext ctx;
                DomainIdHandle domain_id_handle;
                get_domain_handle(_domain_list, domain_id_handle, ctx);
                bool is_sys_registrar;
                std::string sys_registrar;
                for (IdlDomainIdList::const_iterator pDomainId = _domain_list.begin(); pDomainId != _domain_list.end(); ++pDomainId) {
                    const Fred::ObjectId object_id = *pDomainId;
                    try {
                        Fred::ClearAdministrativeObjectStateRequestId(object_id, _reason).exec(ctx);
                        Fred::PerformObjectStateRequest(object_id).exec(ctx);
                        const std::string fqdn = get_object_handle(ctx, object_id);
                        if (sys_registrar.empty()) {
                            sys_registrar = get_sys_registrar(ctx, is_sys_registrar);
                        }
                        const Fred::InfoDomainData info_domain_data = Fred::InfoDomain(fqdn, sys_registrar).exec(ctx).info_domain_data;
                        const boost::gregorian::date today(boost::gregorian::day_clock::universal_day());
                        const bool set_expire_today = info_domain_data.expiration_date < today;
                        const bool set_new_owner = !_new_owner.isnull() && !static_cast< std::string >(_new_owner).empty();
                        if (_remove_admin_c || set_new_owner || set_expire_today) {
                            if (sys_registrar.empty()) {
                                sys_registrar = get_sys_registrar(ctx, is_sys_registrar);
                            }
                            if (!is_sys_registrar) {
                                EX_INTERNAL_SERVER_ERROR e;
                                e.what = "system registrar not found";
                                throw e;
                            }
                            Fred::UpdateDomain update_domain(fqdn, sys_registrar);
                            if (set_new_owner) {
                                update_domain.set_registrant(_new_owner);
                            }
                            if (set_expire_today) {
                                update_domain.set_domain_expiration(today);
                            }
                            if (0 < _log_req_id) {
                                update_domain.set_logd_request_id(_log_req_id);
                            }
                            if (_remove_admin_c) {
                                for (std::vector< std::string >::const_iterator pAdmin = info_domain_data.admin_contacts.begin();
                                     pAdmin != info_domain_data.admin_contacts.end(); ++pAdmin) {
                                    update_domain.rem_admin_contact(*pAdmin);
                                }
                            }
                            update_domain.exec(ctx);
                        }
                    }
                    catch (const Fred::ClearAdministrativeObjectStateRequestId::Exception &e) {
                        if (e.is_set_server_blocked_absent()) {
                            EX_DOMAIN_ID_NOT_BLOCKED::Item e_item;
                            e_item.domain_id = e.get_server_blocked_absent();
                            e_item.domain_handle = domain_id_handle[e_item.domain_id];
                            domain_id_not_blocked.what.insert(e_item);
                        }
                        else if (e.is_set_object_id_not_found()) {
                            EX_INTERNAL_SERVER_ERROR ex;
                            ex.what = "object not found"; 
                            throw ex;
                        }
                        else {
                            throw std::runtime_error("Fred::ClearAdministrativeObjectStateRequestId::Exception");
                        }
                    }
                    catch (const Fred::UpdateDomain::Exception &e) {
                        if (e.is_set_unknown_registrant_handle()) {
                            EX_NEW_OWNER_DOES_NOT_EXISTS ex;
                            ex.what = e.get_unknown_registrant_handle();
                            throw ex;
                        }
                        else {
                            throw std::runtime_error("Fred::UpdateDomain::Exception");
                        }
                    }
                    catch (const Fred::InfoDomain::Exception &e) {
                        if (e.is_set_unknown_domain_fqdn()) {
                            EX_INTERNAL_SERVER_ERROR ex;
                            ex.what = "domain doesn't exist"; 
                            throw ex;
                        }
                        else {
                            throw std::runtime_error("Fred::InfoDomain::Exception");
                        }
                    }
                }
                if (!domain_id_not_blocked.what.empty()) {
                    throw domain_id_not_blocked;
                }
                ctx.commit_transaction();
            }
            catch (const EX_DOMAIN_ID_NOT_FOUND&) {
                throw;
            }
            catch (const EX_DOMAIN_ID_NOT_BLOCKED&) {
                throw;
            }
            catch (const EX_NEW_OWNER_DOES_NOT_EXISTS&) {
                throw;
            }
            catch (const std::exception &e) {
                EX_INTERNAL_SERVER_ERROR ex;
                ex.what = e.what();
                throw ex;
            }
        }

        void BlockingImpl::blacklistAndDeleteDomainsId(
            const IdlDomainIdList &_domain_list,
            const Nullable< boost::gregorian::date > &_blacklist_to_date,
            const std::string &_reason,
            unsigned long long _log_req_id)
        {
            EX_DOMAIN_ID_NOT_FOUND domain_id_not_found;
            try {
                boost::posix_time::ptime blacklist_to_limit;
                if (!_blacklist_to_date.isnull()) {
                    blacklist_to_limit = boost::posix_time::ptime(static_cast< boost::gregorian::date >(_blacklist_to_date),
                                                                  boost::posix_time::time_duration(12, 0, 0));
                }
                Fred::OperationContext ctx;
                for (IdlDomainIdList::const_iterator pDomainId = _domain_list.begin(); pDomainId != _domain_list.end(); ++pDomainId) {
                    const Fred::ObjectId object_id = *pDomainId;
                    Fred::CreateDomainNameBlacklistId create_domain_name_blacklist(object_id, _reason);
                    if (!_blacklist_to_date.isnull()) {
                        create_domain_name_blacklist.set_valid_to(blacklist_to_limit);
                    }
                    try {
                        create_domain_name_blacklist.exec(ctx);
                        Database::Result object_handle_res = ctx.get_conn().exec_params(
                            "SELECT name "
                            "FROM object_registry "
                            "WHERE id=$1::bigint", Database::query_param_list(object_id));
                        if (object_handle_res.size() == 1) {
                            const std::string domain = static_cast< std::string >(object_handle_res[0][0]);
                            Fred::DeleteDomain(domain).exec(ctx);
                        }
                    }
                    catch (const Fred::CreateDomainNameBlacklistId::Exception &e) {
                        if (e.is_set_object_id_not_found()) {
                            domain_id_not_found.what.insert(e.get_object_id_not_found());
                        }
                        else {
                            throw;
                        }
                    }
                    catch (const Fred::DeleteDomain::Exception &e) {
                        if (e.is_set_unknown_domain_fqdn()) {
                            domain_id_not_found.what.insert(object_id);
                        }
                        else {
                            throw;
                        }
                    }
                }
                if (!domain_id_not_found.what.empty()) {
                    throw domain_id_not_found;
                }
                ctx.commit_transaction();
            }
            catch (const EX_DOMAIN_ID_NOT_FOUND&) {
                throw;
            }
            catch (const std::exception &e) {
                EX_INTERNAL_SERVER_ERROR ex;
                ex.what = e.what();
                throw ex;
            }
        }

        void BlockingImpl::blacklistDomainsId(
            const IdlDomainIdList &_domain_list,
            const Nullable< boost::gregorian::date > &_blacklist_to_date,
            bool _with_delete,
            unsigned long long _log_req_id)
        {
            try {
                boost::posix_time::ptime blacklist_to_limit;
                if (!_blacklist_to_date.isnull()) {
                    blacklist_to_limit = boost::posix_time::ptime(static_cast< boost::gregorian::date >(_blacklist_to_date),
                                                                  boost::posix_time::time_duration(12, 0, 0));
                }
                Fred::OperationContext ctx;
                for (IdlDomainIdList::const_iterator pDomainId = _domain_list.begin(); pDomainId != _domain_list.end(); ++pDomainId) {
                    const Fred::ObjectId object_id = *pDomainId;
                    Fred::CreateDomainNameBlacklistId create_domain_name_blacklist(object_id, "blacklistDomainsId() call");
                    if (!_blacklist_to_date.isnull()) {
                        create_domain_name_blacklist.set_valid_to(blacklist_to_limit);
                    }
                    create_domain_name_blacklist.exec(ctx);
                }
                ctx.commit_transaction();
            }
            catch (const EX_DOMAIN_ID_NOT_FOUND&) {
                throw;
            }
            catch (const std::exception &e) {
                EX_INTERNAL_SERVER_ERROR ex;
                ex.what = e.what();
                throw ex;
            }
        }

//        void BlockingImpl::unblacklistAndCreateDomains(
//            const ::Registry::Administrative::DomainList &_domain_list,
//            const std::string &_owner)
//        {
//        }

    }//namespace Administrative
}//namespace Registry



