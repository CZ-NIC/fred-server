/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
 *  @administrativeblocking.cc
 *  administrativeblocking implementation
 */

#include "src/backend/admin_block/administrativeblocking.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_registry_args.hh"

#include "libfred/object/object_state.hh"
#include "libfred/object/object_type.hh"
#include "libfred/object_state/clear_admin_object_state_request_id.hh"
#include "libfred/object_state/create_admin_object_block_request_id.hh"
#include "libfred/object_state/create_admin_object_state_restore_request_id.hh"
#include "libfred/object_state/get_blocking_status_desc_list.hh"
#include "libfred/object_state/get_object_state_id_map.hh"
#include "libfred/object_state/get_object_states.hh"
#include "libfred/object_state/object_has_state.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/poll/create_update_object_poll_message.hh"
#include "libfred/registrable_object/contact/copy_contact.hh"
#include "libfred/registrable_object/domain/create_domain_name_blacklist_id.hh"
#include "libfred/registrable_object/domain/delete_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/update_domain.hh"
#include "libfred/registrar/info_registrar.hh"

#include "util/log/context.hh"
#include "util/random/random.hh"

#include <map>
#include <memory>
#include <ostream>

namespace {

enum
{
    OBJECT_TYPE_DOMAIN = 3
};

std::string create_ctx_name(const std::string& _name)
{
    return str(boost::format("%1%-<%2%>") % _name % Random::Generator().get(0, 10000));
}

std::string get_system_registrar()
{
    const std::string registrar_handle =
            CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>()->system_registrar;
    if (registrar_handle.empty())
    {
        std::ostringstream debug_info;
        debug_info << "missing configuration for system registrar";
        throw std::runtime_error(debug_info.str());
    }
    try
    {
        LibFred::OperationContextCreator ctx;
        const unsigned long long registrar_id =
                LibFred::InfoRegistrarByHandle(registrar_handle).exec(ctx).info_registrar_data.id;
        if (registrar_id > 0)
        {
            return registrar_handle;
        }
    }
    catch (const LibFred::InfoRegistrarByHandle::Exception& e)
    {
        if (e.is_set_unknown_registrar_handle())
        {
            std::ostringstream debug_info;
            debug_info << "registrar with handle " << registrar_handle << " not found in database";
            throw std::runtime_error(debug_info.str());
        }
        throw;
    }
    std::ostringstream debug_info;
    debug_info << "registrar with handle " << registrar_handle << " is not a system registrar";
    throw std::runtime_error(debug_info.str());
}

} // namespace {anonymous}

namespace Fred {
namespace Backend {
namespace AdministrativeBlocking {

BlockingImpl::BlockingImpl(const std::string& _server_name)
    : server_name_(_server_name),
      system_registrar_(get_system_registrar())
{
}

LibFred::GetBlockingStatusDescList::StatusDescList BlockingImpl::getBlockingStatusDescList(const std::string& _lang)
{
    Logging::Context ctx_server(create_ctx_name(this->get_server_name()));
    Logging::Context ctx_method("get-blocking-status-desc-list");
    try
    {
        LibFred::OperationContextCreator ctx;
        LibFred::GetBlockingStatusDescList blocking_status_desc_list;
        blocking_status_desc_list.set_lang(_lang);
        LibFred::GetBlockingStatusDescList::StatusDescList& desc_list = blocking_status_desc_list.exec(ctx);
        ctx.commit_transaction();
        return desc_list;
    }
    catch (const std::exception& e)
    {
        EX_INTERNAL_SERVER_ERROR ex;
        ex.what = e.what();
        throw ex;
    }
}

namespace {

struct OwnerCopy
{
    std::string old_owner_handle;
    LibFred::ObjectId new_owner_id;
    std::string new_owner_handle;
};

typedef std::map<LibFred::ObjectId, LibFred::ObjectId> DomainIdOwnerId;
typedef std::map<LibFred::ObjectId, OwnerCopy> OwnerIdOwnerCopy;
typedef std::map<LibFred::ObjectId, std::string> DomainIdHandle;

static const std::string owner_copy_suffix = "-ABC_"; //AdministrativeBlockingCopy
static const std::string owner_copy_zero_idx = "00000";

std::string get_copy_owner_handle(
        const std::string& _owner_handle,
        unsigned long long _log_req_id,
        LibFred::OperationContext& _ctx)
{
    std::string new_owner_handle = _owner_handle + owner_copy_suffix;
    Database::Result last_owner_res = _ctx.get_conn().exec_params(
                    // clang-format off
                    "SELECT name "
                    "FROM object_registry "
                    "WHERE (type=$1::bigint) AND "
                          "($2::text<name) AND "
                          "(name LIKE $3::text) AND "
                          "erdate IS NULL "
                    "ORDER BY name DESC LIMIT 1", Database::query_param_list(LibFred::CopyContact::OBJECT_TYPE_ID_CONTACT)
                                                                            (new_owner_handle)
                                                                            (new_owner_handle + "%"));
                    // clang-format on

    if (last_owner_res.size() == 0)
    {
        new_owner_handle += owner_copy_zero_idx;
        return new_owner_handle;
    }
    const std::string last_owner_name = static_cast<std::string>(last_owner_res[0][0]);
    const std::string str_copy_idx = last_owner_name.substr(new_owner_handle.length());
    ::size_t copy_idx = 0;
    for (const char* pC = str_copy_idx.c_str(); *pC != '\0'; ++pC)
    {
        const char c = *pC;
        if ((c < '0') || ('9' < c))
        {
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

void copy_domain_owners(
        const std::string& _sys_registrar_handle,
        const IdlDomainIdList& _domain_list,
        DomainIdOwnerId& _domain_id_owner_id,
        OwnerIdOwnerCopy& _owner_id_owner_copy,
        unsigned long long _log_req_id,
        LibFred::OperationContext& _ctx)
{
    std::ostringstream query;
    for (IdlDomainIdList::const_iterator pObjectId = _domain_list.begin(); pObjectId != _domain_list.end(); ++pObjectId)
    {
        if (query.str().empty())
        {
            query << "SELECT d.id,d.registrant,obr.name "
                     "FROM domain d "
                     "JOIN object_registry obr ON obr.id=d.registrant "
                     "WHERE d.id IN (";
        }
        else
        {
            query << ",";
        }
        query << (*pObjectId);
    }
    query << ") FOR UPDATE OF obr";
    Database::Result domain_info_res = _ctx.get_conn().exec(query.str());
    for (unsigned idx = 0; idx < domain_info_res.size(); ++idx)
    {
        OwnerCopy item;
        const LibFred::ObjectId old_owner_id = static_cast<LibFred::ObjectId>(domain_info_res[idx][1]);
        if (_owner_id_owner_copy.find(old_owner_id) == _owner_id_owner_copy.end())
        {
            _owner_id_owner_copy[old_owner_id].old_owner_handle = static_cast<std::string>(domain_info_res[idx][2]);
        }
        const LibFred::ObjectId domain_id = static_cast<LibFred::ObjectId>(domain_info_res[idx][0]);
        _domain_id_owner_id[domain_id] = old_owner_id;
    }
    for (OwnerIdOwnerCopy::iterator pOwnerCopy = _owner_id_owner_copy.begin(); pOwnerCopy != _owner_id_owner_copy.end(); ++pOwnerCopy)
    {
        pOwnerCopy->second.new_owner_handle = get_copy_owner_handle(pOwnerCopy->second.old_owner_handle, _log_req_id, _ctx);
        LibFred::CopyContact copy_contact(pOwnerCopy->second.old_owner_handle,
                pOwnerCopy->second.new_owner_handle,
                _sys_registrar_handle,
                _log_req_id);
        pOwnerCopy->second.new_owner_id = copy_contact.exec(_ctx);
    }
}

void check_owner_has_no_other_domains(
        const IdlDomainIdList& _domain_list,
        LibFred::OperationContext& _ctx)
{
    if (_domain_list.empty())
    {
        return;
    }
    IdlDomainIdList::const_iterator pDomainId = _domain_list.begin();
    Database::query_param_list param(*pDomainId);
    std::ostringstream set_of_domain_id;
    set_of_domain_id << "($" << param.size() << "::bigint";
    for (++pDomainId; pDomainId != _domain_list.end(); ++pDomainId)
    {
        param(*pDomainId);
        set_of_domain_id << ",$" << param.size() << "::bigint";
    }
    set_of_domain_id << ")";
    Database::Result check_res = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT d.id,dh.name,d.registrant,oh.name "
            "FROM domain d "
            "JOIN object_registry dh ON dh.id=d.id "
            "JOIN object_registry oh ON oh.id=d.registrant "
            "WHERE d.registrant IN (SELECT registrant "
            "FROM domain "
            "WHERE id IN " +
                    set_of_domain_id.str() + " GROUP BY registrant) AND "
                                             "d.id NOT IN " +
                    set_of_domain_id.str(),
            // clang-format on
            param);
    if (check_res.size() <= 0)
    {
        return;
    }
    EX_OWNER_HAS_OTHER_DOMAIN ex;
    for (unsigned idx = 0; idx < check_res.size(); ++idx)
    {
        EX_DOMAIN_ID_ALREADY_BLOCKED::Item domain;
        domain.domain_id = static_cast<LibFred::ObjectId>(check_res[idx][0]);
        domain.domain_handle = static_cast<std::string>(check_res[idx][1]);
        const LibFred::ObjectId owner_id = static_cast<LibFred::ObjectId>(check_res[idx][2]);
        EX_OWNER_HAS_OTHER_DOMAIN::Type::iterator pItem = ex.what.find(owner_id);
        if (pItem == ex.what.end())
        {
            EX_OWNER_HAS_OTHER_DOMAIN::Item item;
            item.owner_handle = static_cast<std::string>(check_res[idx][3]);
            pItem = ex.what.insert(std::make_pair(owner_id, item)).first;
        }
        pItem->second.domain.insert(domain);
    }
    throw ex;
}

void check_owner_is_blockable(
        const IdlDomainIdList& _domain_list,
        LibFred::OperationContext& _ctx)
{
    if (_domain_list.empty())
    {
        return;
    }
    IdlDomainIdList::const_iterator pDomainId = _domain_list.begin();
    Database::query_param_list param(*pDomainId);
    std::ostringstream set_of_domain_id;
    set_of_domain_id << "($" << param.size() << "::bigint";
    for (++pDomainId; pDomainId != _domain_list.end(); ++pDomainId)
    {
        param(*pDomainId);
        set_of_domain_id << ",$" << param.size() << "::bigint";
    }
    set_of_domain_id << ")";
    Database::Result check_res = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT obr.id,obr.name "
            "FROM domain d "
            "JOIN object_registry obr ON (obr.id=d.registrant AND obr.type=1) "
            "JOIN object_state os ON os.object_id=d.registrant "
            "JOIN enum_object_states eos ON (eos.id=os.state_id AND eos.name='mojeidContact') "
            "WHERE obr.erdate IS NULL AND "
            "os.valid_from<=CURRENT_TIMESTAMP AND (CURRENT_TIMESTAMP<os.valid_to OR os.valid_to IS NULL) AND "
            "d.id IN " +
                    set_of_domain_id.str() + " "
                                             "GROUP BY obr.id,obr.name",
            // clang-format on
            param);
    if (check_res.size() <= 0)
    {
        return;
    }
    EX_CONTACT_BLOCK_PROHIBITED ex;
    for (unsigned idx = 0; idx < check_res.size(); ++idx)
    {
        EX_CONTACT_BLOCK_PROHIBITED::Item contact;
        contact.contact_id = static_cast<LibFred::ObjectId>(check_res[idx][0]);
        contact.contact_handle = static_cast<std::string>(check_res[idx][1]);
        ex.what.insert(contact);
    }
    throw ex;
}

DomainIdHandle& get_domain_handle(
        const IdlDomainIdList& _domain_list,
        DomainIdHandle& _result,
        LibFred::OperationContext& _ctx)
{
    _result.clear();
    if (_domain_list.empty())
    {
        return _result;
    }
    std::ostringstream query;
    IdlDomainIdList::const_iterator pDomainId = _domain_list.begin();
    Database::query_param_list param(*pDomainId);
    query << "SELECT obr.id,obr.name "
             "FROM object_registry obr "
             "JOIN domain d ON d.id=obr.id "
             "WHERE obr.id IN ($"
          << param.size() << "::bigint";
    for (++pDomainId; pDomainId != _domain_list.end(); ++pDomainId)
    {
        param(*pDomainId);
        query << ",$" << param.size() << "::bigint";
    }
    query << ") AND obr.erdate IS NULL "
             "FOR UPDATE OF obr";
    Database::Result domain_id_handle_res = _ctx.get_conn().exec_params(query.str(), param);
    for (::size_t idx = 0; idx < domain_id_handle_res.size(); ++idx)
    {
        const Database::Row& row = domain_id_handle_res[idx];
        _result[static_cast<LibFred::ObjectId>(row[0])] = static_cast<std::string>(row[1]);
    }
    if (_domain_list.size() == _result.size())
    {
        return _result;
    }
    EX_DOMAIN_ID_NOT_FOUND e;
    for (IdlDomainIdList::const_iterator pDomainId = _domain_list.begin();
            pDomainId != _domain_list.end();
            ++pDomainId)
    {
        if (_result.find(*pDomainId) == _result.end())
        {
            e.what.insert(*pDomainId);
        }
    }
    throw e;
}

IdlDomainIdList get_blocked_domains_by_owner(LibFred::OperationContext& _ctx,
        const LibFred::ObjectId& _owner_id)
{
    IdlDomainIdList blocked_domain_list;
    const std::string type_of_object = Conversion::Enums::to_db_handle(LibFred::Object_Type::contact);
    const std::string type_of_state = Conversion::Enums::to_db_handle(LibFred::Object_State::server_blocked);
    Database::Result db_res = _ctx.get_conn().exec_params(
            // clang-format off
            "SELECT d.id "
            "FROM domain d "
            "JOIN object_registry obr ON obr.id = d.registrant AND obr.type = get_object_type_id($1::text) "
            "JOIN object_state os ON os.object_id = d.id "
            "JOIN enum_object_states eos ON (eos.id = os.state_id AND eos.name = $2::text) "
            "WHERE obr.erdate IS NULL "
            "AND os.valid_to IS NULL "
            "AND obr.id=$3::integer ",
            // clang-format on
            Database::query_param_list(type_of_object)(type_of_state)(_owner_id));

    for (unsigned i = 0; i < db_res.size(); ++i)
    {
        blocked_domain_list.insert(static_cast<unsigned long long>(db_res[i][0]));
    }
    return blocked_domain_list;
}

void administrative_unblock_contact(
        LibFred::OperationContext& _ctx,
        const std::string& _system_registrar,
        const LibFred::ObjectId _contact_id,
        const std::string& _reason,
        unsigned long long _log_req_id,
        bool _create_copy_contact_allowed = false)
{
    const bool blocked_contact = LibFred::ObjectHasState(_contact_id, LibFred::Object_State::server_blocked)
            .exec(_ctx);
    if (!blocked_contact)
    {
        return;
    }

    std::ostringstream debug_info;
    debug_info << "Unblock contact: " << _contact_id;
    LOGGER.debug(debug_info.str());
    const IdlDomainIdList blocked_domain_ids = get_blocked_domains_by_owner(_ctx, _contact_id);
    if (blocked_domain_ids.size() > 0)
    {
        if (_create_copy_contact_allowed)
        {
            LOGGER.debug("Contact has some blocked domain - create copy of contact");
            DomainIdOwnerId domain_id_owner_id;
            OwnerIdOwnerCopy owner_id_owner_copy;

            copy_domain_owners(_system_registrar,
                    blocked_domain_ids,
                    domain_id_owner_id,
                    owner_id_owner_copy,
                    _log_req_id,
                    _ctx);

            for (const auto owner : owner_id_owner_copy)
            {
                const std::vector<LibFred::ObjectStateData> object_states =
                        LibFred::GetObjectStates(owner.first).exec(_ctx);
                LibFred::StatusList status_list;
                std::for_each(object_states.begin(), object_states.end(), [&status_list](const auto _s) {
                        if (_s.is_manual && _s.state_name != "serverBlocked")
                        {
                        status_list.insert(_s.state_name);
                        }
                        });
                LibFred::CreateAdminObjectBlockRequestId create_object_block_request(owner.second.new_owner_id, status_list);
                create_object_block_request.set_reason(_reason);
                create_object_block_request.exec(_ctx);
                LibFred::PerformObjectStateRequest(owner.second.new_owner_id).exec(_ctx);
            }

            DomainIdHandle blocked_domains;
            get_domain_handle(blocked_domain_ids, blocked_domains, _ctx);
            const std::string new_owner_handle = owner_id_owner_copy[_contact_id].new_owner_handle;
            for (const auto domain : blocked_domains)
            {
                std::ostringstream update_info;
                update_info << "Update domain " << domain.second << " - set new owner: " << new_owner_handle;

                LibFred::UpdateDomain update_domain(domain.second, _system_registrar);
                update_domain.set_registrant(new_owner_handle);
                if (_log_req_id > 0)
                {
                    update_domain.set_logd_request_id(_log_req_id);
                }
                LOGGER.debug(update_info.str());
                update_domain.exec(_ctx);
            }
        }
        else
        {
            LOGGER.debug("Unblock contact is not possible - contact has some blocked domain");
            return;
        }
    }
    LibFred::CreateAdminObjectStateRestoreRequestId create_object_state_restore_request(_contact_id, _reason, _log_req_id);
    create_object_state_restore_request.exec(_ctx);
    return;
}

void administrative_unblock_domains(
        const IdlDomainIdList& _domain_list,
        const Nullable<std::string>& _new_owner,
        const std::string& _system_registrar,
        bool _remove_admin_c,
        const std::string& _reason,
        unsigned long long _log_req_id,
        bool _restore_prev_state)
{
    EX_DOMAIN_ID_NOT_BLOCKED domain_id_not_blocked;
    try
    {
        LibFred::OperationContextCreator ctx;
        DomainIdHandle domain_id_handle;
        std::set<unsigned long long> owners;
        const boost::gregorian::date today(boost::gregorian::day_clock::universal_day());
        const bool set_new_owner = !_new_owner.isnull() && !_new_owner.get_value().empty();

        get_domain_handle(_domain_list, domain_id_handle, ctx);

        for (const auto object_id : _domain_list)
        {
            try
            {
                LOGGER.debug("Unblock domain " + domain_id_handle[object_id]);
                if (_restore_prev_state)
                {
                    LibFred::CreateAdminObjectStateRestoreRequestId create_object_state_restore_request(
                            object_id,
                            _reason,
                            _log_req_id);
                    create_object_state_restore_request.exec(ctx);
                }
                else
                {
                    LibFred::ClearAdminObjectStateRequestId(object_id, _reason).exec(ctx);
                }

                const LibFred::InfoDomainData info_domain_data =
                        LibFred::InfoDomainById(object_id).exec(ctx).info_domain_data;
                owners.insert(info_domain_data.registrant.id);

                const bool set_expire_today = info_domain_data.expiration_date < today;
                if (_remove_admin_c || set_new_owner || set_expire_today)
                {
                    LibFred::UpdateDomain update_domain(info_domain_data.fqdn, _system_registrar);
                    if (set_new_owner)
                    {
                        update_domain.set_registrant(_new_owner.get_value());
                    }
                    if (set_expire_today)
                    {
                        update_domain.set_domain_expiration(today);
                    }
                    if (0 < _log_req_id)
                    {
                        update_domain.set_logd_request_id(_log_req_id);
                    }
                    if (_remove_admin_c)
                    {
                        for (const auto& admin_contact : info_domain_data.admin_contacts)
                        {
                            update_domain.rem_admin_contact(admin_contact.handle);
                        }
                    }
                    //domain expiration has to be set before update_object_states invocation
                    const unsigned long long new_hid = update_domain.exec(ctx);
                    if (!_restore_prev_state)
                    {
                        /* in case of error when creating poll message we fail with internal server error */
                        LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, new_hid);
                    }
                }
                //update_object_states invocation
                LibFred::PerformObjectStateRequest(object_id).exec(ctx);
            }
            catch (const LibFred::CreateAdminObjectStateRestoreRequestId::Exception& e)
            {
                if (e.is_set_server_blocked_absent())
                {
                    EX_DOMAIN_ID_NOT_BLOCKED::Item e_item;
                    e_item.domain_id = e.get_server_blocked_absent();
                    e_item.domain_handle = domain_id_handle[e_item.domain_id];
                    domain_id_not_blocked.what.insert(e_item);
                }
                else
                {
                    throw;
                }
            }
            catch (const LibFred::ClearAdminObjectStateRequestId::Exception& e)
            {
                if (e.is_set_server_blocked_absent())
                {
                    EX_DOMAIN_ID_NOT_BLOCKED::Item e_item;
                    e_item.domain_id = e.get_server_blocked_absent();
                    e_item.domain_handle = domain_id_handle[e_item.domain_id];
                    domain_id_not_blocked.what.insert(e_item);
                }
                else if (e.is_set_object_id_not_found())
                {
                    EX_INTERNAL_SERVER_ERROR ex;
                    ex.what = "object not found";
                    throw ex;
                }
                else
                {
                    throw std::runtime_error("LibFred::ClearAdminObjectStateRequestId::Exception");
                }
            }
            catch (const LibFred::UpdateDomain::Exception& e)
            {
                if (e.is_set_unknown_registrant_handle())
                {
                    EX_NEW_OWNER_DOES_NOT_EXISTS ex;
                    ex.what = e.get_unknown_registrant_handle();
                    throw ex;
                }
                else
                {
                    throw std::runtime_error("LibFred::UpdateDomain::Exception");
                }
            }
            catch (const LibFred::InfoDomainByFqdn::Exception& e)
            {
                if (e.is_set_unknown_fqdn())
                {
                    EX_INTERNAL_SERVER_ERROR ex;
                    ex.what = "domain doesn't exist";
                    throw ex;
                }
                else
                {
                    throw std::runtime_error("LibFred::InfoDomain::Exception");
                }
            }
        }
        if (!domain_id_not_blocked.what.empty())
        {
            throw domain_id_not_blocked;
        }
        const bool create_copy_contact_allowed = !set_new_owner;
        for (const auto owner_id : owners)
        {
            std::ostringstream debug_info;
            debug_info << "Restore previous state of contact: " << owner_id;
            LOGGER.debug(debug_info.str());
            administrative_unblock_contact(ctx,
                    _system_registrar,
                    owner_id,
                    _reason,
                    _log_req_id,
                    create_copy_contact_allowed);
            LibFred::PerformObjectStateRequest(owner_id).exec(ctx);
        }
        ctx.commit_transaction();
    }
    catch (const EX_DOMAIN_ID_NOT_FOUND&)
    {
        throw;
    }
    catch (const EX_DOMAIN_ID_NOT_BLOCKED&)
    {
        throw;
    }
    catch (const EX_NEW_OWNER_DOES_NOT_EXISTS&)
    {
        throw;
    }
    catch (const std::exception& e)
    {
        EX_INTERNAL_SERVER_ERROR ex;
        ex.what = e.what();
        LOGGER.error(e.what());
        throw ex;
    }
}

} // namespace Fred::Backend::AdministrativeBlocking::{anonymous}

IdlOwnerChangeList BlockingImpl::blockDomainsId(
        const IdlDomainIdList& _domain_list,
        const LibFred::StatusList& _status_list,
        IdlOwnerBlockMode _owner_block_mode,
        const Nullable<boost::gregorian::date>& _block_to_date,
        const std::string& _reason,
        unsigned long long _log_req_id)
{
    Logging::Context ctx_server(create_ctx_name(this->get_server_name()));
    Logging::Context ctx_method("block-domains-id");

    EX_DOMAIN_ID_NOT_FOUND domain_id_not_found;
    EX_UNKNOWN_STATUS unknown_status;
    EX_DOMAIN_ID_ALREADY_BLOCKED domain_id_already_blocked;
    EX_CONTACT_BLOCK_PROHIBITED contact_block_prohibited;
    try
    {
        IdlOwnerChangeList result;
        LibFred::OperationContextCreator ctx;
        DomainIdHandle domain_id_handle;
        get_domain_handle(_domain_list, domain_id_handle, ctx);
        LibFred::StatusList contact_status_list;
        DomainIdOwnerId domain_id_owner_id;
        OwnerIdOwnerCopy owner_id_owner_copy;
        if ((_owner_block_mode == OWNER_BLOCK_MODE_BLOCK_OWNER) ||
                (_owner_block_mode == OWNER_BLOCK_MODE_BLOCK_OWNER_COPY))
        {
            LibFred::GetObjectStateIdMap::StateIdMap state_id;
            LibFred::GetObjectStateIdMap::get_result(ctx, _status_list, LibFred::CopyContact::OBJECT_TYPE_ID_CONTACT, state_id);
            for (LibFred::GetObjectStateIdMap::StateIdMap::const_iterator pState = state_id.begin();
                    pState != state_id.end();
                    ++pState)
            {
                contact_status_list.insert(pState->first);
            }
            if (_owner_block_mode == OWNER_BLOCK_MODE_BLOCK_OWNER_COPY)
            {
                copy_domain_owners(system_registrar_, _domain_list, domain_id_owner_id, owner_id_owner_copy, _log_req_id, ctx);
            }
            else if (_owner_block_mode == OWNER_BLOCK_MODE_BLOCK_OWNER)
            {
                check_owner_has_no_other_domains(_domain_list, ctx);
                check_owner_is_blockable(_domain_list, ctx);
            }
        }
        StringSet contact_blocked;
        boost::posix_time::ptime block_time_limit;
        if (!_block_to_date.isnull())
        {
            // block to date 12:00:00 https://admin.nic.cz/ticket/6314#comment:50
            block_time_limit = boost::posix_time::ptime(_block_to_date.get_value(),
                    boost::posix_time::time_duration(12, 0, 0));
        }
        for (IdlDomainIdList::const_iterator pObjectId = _domain_list.begin(); pObjectId != _domain_list.end(); ++pObjectId)
        {
            try
            {
                const LibFred::ObjectId object_id = *pObjectId;
                LibFred::CreateAdminObjectBlockRequestId create_object_block_request(object_id, _status_list);
                create_object_block_request.set_reason(_reason);
                if (!_block_to_date.isnull())
                {
                    create_object_block_request.set_valid_to(block_time_limit);
                }
                if (_owner_block_mode == OWNER_BLOCK_MODE_BLOCK_OWNER)
                {
                    Database::query_param_list param(object_id);
                    Database::Result registrant_result = ctx.get_conn().exec_params(
                            "SELECT rc.id,rc.name "
                            "FROM domain d "
                            "JOIN object_registry rc ON rc.id=d.registrant "
                            "WHERE d.id=$1::bigint "
                            "FOR UPDATE OF rc",
                            param);
                    if (registrant_result.size() <= 0)
                    {
                        domain_id_not_found.what.insert(object_id);
                        continue;
                    }
                    const Database::Row& row = registrant_result[0];
                    const LibFred::ObjectId registrant_id = static_cast<LibFred::ObjectId>(row[0]);
                    const std::string registrant = static_cast<std::string>(row[1]);
                    if ((contact_blocked.find(registrant) == contact_blocked.end()) && !contact_status_list.empty())
                    {
                        contact_blocked.insert(registrant);
                        LibFred::CreateAdminObjectBlockRequestId block_owner_request(registrant_id, contact_status_list);
                        block_owner_request.set_reason(_reason);
                        if (!_block_to_date.isnull())
                        {
                            block_owner_request.set_valid_to(block_time_limit);
                        }
                        block_owner_request.exec(ctx);
                        LibFred::PerformObjectStateRequest(registrant_id).exec(ctx);
                    }
                    create_object_block_request.exec(ctx);
                }
                else if (_owner_block_mode == OWNER_BLOCK_MODE_BLOCK_OWNER_COPY)
                {
                    const std::string domain = create_object_block_request.exec(ctx);
                    IdlOwnerChange result_item;
                    result_item.domain_id = object_id;
                    result_item.domain_handle = domain;
                    result_item.old_owner_id = domain_id_owner_id[object_id];
                    const OwnerCopy& owner_copy = owner_id_owner_copy[result_item.old_owner_id];
                    result_item.old_owner_handle = owner_copy.old_owner_handle;
                    result_item.new_owner_handle = owner_copy.new_owner_handle;
                    result_item.new_owner_id = owner_copy.new_owner_id;
                    if (!contact_status_list.empty())
                    {
                        LibFred::UpdateDomain update_domain(domain, system_registrar_);
                        update_domain.set_registrant(result_item.new_owner_handle);
                        if (0 < _log_req_id)
                        {
                            update_domain.set_logd_request_id(_log_req_id);
                        }
                        const unsigned long long new_hid = update_domain.exec(ctx);
                        /* in case of error when creating poll message we fail with internal server error */
                        LibFred::Poll::CreateUpdateObjectPollMessage().exec(ctx, new_hid);
                    }
                    result.push_back(result_item);
                }
                else
                { // KEEP_OWNER
                    create_object_block_request.exec(ctx);
                }
                LibFred::PerformObjectStateRequest(object_id).exec(ctx);
            }
            catch (const LibFred::CreateObjectStateRequestId::Exception& e)
            {
                if (e.is_set_object_id_not_found())
                {
                    domain_id_not_found.what.insert(e.get_object_id_not_found());
                }
                else if (e.is_set_vector_of_state_not_found())
                {
                    std::vector<std::string> state_not_found = e.get_vector_of_state_not_found();
                    for (std::vector<std::string>::const_iterator pStat = state_not_found.begin();
                            pStat != state_not_found.end();
                            ++pStat)
                    {
                        unknown_status.what.insert(*pStat);
                    }
                }
                else
                {
                    throw std::runtime_error("LibFred::CreateObjectStateRequestId::Exception");
                }
            }
            catch (const LibFred::CreateAdminObjectBlockRequestId::Exception& e)
            {
                if (e.is_set_server_blocked_present())
                {
                    EX_DOMAIN_ID_ALREADY_BLOCKED::Item e_item;
                    e_item.domain_id = e.get_server_blocked_present();
                    e_item.domain_handle = domain_id_handle[e_item.domain_id];
                    domain_id_already_blocked.what.insert(e_item);
                }
                else if (e.is_set_vector_of_state_not_found())
                {
                    std::vector<std::string> state_not_found = e.get_vector_of_state_not_found();
                    for (std::vector<std::string>::const_iterator pStat = state_not_found.begin();
                            pStat != state_not_found.end();
                            ++pStat)
                    {
                        unknown_status.what.insert(*pStat);
                    }
                }
                else
                {
                    throw std::runtime_error("LibFred::CreateAdminObjectBlockRequestId::Exception");
                }
            }
        }
        if (_owner_block_mode == OWNER_BLOCK_MODE_BLOCK_OWNER_COPY)
        {
            for (OwnerIdOwnerCopy::const_iterator pOwnerCopy = owner_id_owner_copy.begin();
                    pOwnerCopy != owner_id_owner_copy.end();
                    ++pOwnerCopy)
            {
                LibFred::CreateAdminObjectBlockRequestId block_owner_request(pOwnerCopy->second.new_owner_id,
                        contact_status_list);
                block_owner_request.set_reason(_reason);
                if (!_block_to_date.isnull())
                {
                    block_owner_request.set_valid_to(block_time_limit);
                }
                block_owner_request.exec(ctx);
                LibFred::PerformObjectStateRequest(pOwnerCopy->second.new_owner_id).exec(ctx);
            }
        }
        if (!domain_id_not_found.what.empty())
        {
            throw domain_id_not_found;
        }
        if (!domain_id_already_blocked.what.empty())
        {
            throw domain_id_already_blocked;
        }
        if (!unknown_status.what.empty())
        {
            throw unknown_status;
        }
        ctx.commit_transaction();
        return result;
    }
    catch (const EX_DOMAIN_ID_NOT_FOUND&)
    {
        throw;
    }
    catch (const EX_UNKNOWN_STATUS&)
    {
        throw;
    }
    catch (const EX_DOMAIN_ID_ALREADY_BLOCKED&)
    {
        throw;
    }
    catch (const EX_OWNER_HAS_OTHER_DOMAIN&)
    {
        throw;
    }
    catch (const EX_CONTACT_BLOCK_PROHIBITED&)
    {
        throw;
    }
    catch (const std::exception& e)
    {
        EX_INTERNAL_SERVER_ERROR ex;
        ex.what = e.what();
        LOGGER.info(e.what());
        throw ex;
    }
}

void BlockingImpl::restorePreAdministrativeBlockStatesId(
        const IdlDomainIdList& _domain_list,
        const Nullable<std::string>& _new_owner,
        const std::string& _reason,
        unsigned long long _log_req_id)
{
    Logging::Context ctx_server(create_ctx_name(this->get_server_name()));
    Logging::Context ctx_method("restore-pre-administrative-block-states-id");

    administrative_unblock_domains(_domain_list,
            _new_owner,
            system_registrar_,
            false,
            _reason,
            _log_req_id,
            true);
}

void BlockingImpl::updateBlockDomainsId(
        const IdlDomainIdList& _domain_list,
        const LibFred::StatusList& _status_list,
        const Nullable<boost::gregorian::date>& _block_to_date,
        const std::string& _reason,
        unsigned long long _log_req_id)
{
    Logging::Context ctx_server(create_ctx_name(this->get_server_name()));
    Logging::Context ctx_method("update-block-domains-id");

    EX_DOMAIN_ID_NOT_FOUND domain_id_not_found;
    EX_UNKNOWN_STATUS unknown_status;
    try
    {
        boost::posix_time::ptime block_time_limit;
        if (!_block_to_date.isnull())
        {
            // block to date 12:00:00 https://admin.nic.cz/ticket/6314#comment:50
            block_time_limit = boost::posix_time::ptime(_block_to_date.get_value(),
                    boost::posix_time::time_duration(12, 0, 0));
        }
        LibFred::OperationContextCreator ctx;
        for (IdlDomainIdList::const_iterator pDomainId = _domain_list.begin(); pDomainId != _domain_list.end(); ++pDomainId)
        {
            const LibFred::ObjectId object_id = *pDomainId;
            try
            {
                LibFred::CreateAdminObjectStateRestoreRequestId create_object_state_restore_request(object_id, _reason, _log_req_id);
                create_object_state_restore_request.exec(ctx);
                LibFred::PerformObjectStateRequest(object_id).exec(ctx);
                LibFred::CreateAdminObjectBlockRequestId create_object_state_request(object_id, _status_list);
                if (!_block_to_date.isnull())
                {
                    create_object_state_request.set_valid_to(block_time_limit);
                }
                create_object_state_request.set_reason(_reason);
                create_object_state_request.exec(ctx);
                LibFred::PerformObjectStateRequest(object_id).exec(ctx);
            }
            catch (const LibFred::CreateObjectStateRequestId::Exception& e)
            {
                if (e.is_set_object_id_not_found())
                {
                    domain_id_not_found.what.insert(e.get_object_id_not_found());
                }
                else if (e.is_set_vector_of_state_not_found())
                {
                    std::vector<std::string> state_not_found = e.get_vector_of_state_not_found();
                    for (std::vector<std::string>::const_iterator pStat = state_not_found.begin();
                            pStat != state_not_found.end();
                            ++pStat)
                    {
                        unknown_status.what.insert(*pStat);
                    }
                }
                else
                {
                    throw std::runtime_error("LibFred::CreateObjectStateRequestId::Exception");
                }
            }
            catch (const LibFred::CreateAdminObjectBlockRequestId::Exception& e)
            {
                if (e.is_set_vector_of_state_not_found())
                {
                    std::vector<std::string> state_not_found = e.get_vector_of_state_not_found();
                    for (std::vector<std::string>::const_iterator pStat = state_not_found.begin();
                            pStat != state_not_found.end();
                            ++pStat)
                    {
                        unknown_status.what.insert(*pStat);
                    }
                }
                else
                {
                    throw std::runtime_error("LibFred::CreateAdminObjectBlockRequestId::Exception");
                }
            }
        }
        if (!domain_id_not_found.what.empty())
        {
            throw domain_id_not_found;
        }
        if (!unknown_status.what.empty())
        {
            throw unknown_status;
        }
        ctx.commit_transaction();
    }
    catch (const EX_DOMAIN_ID_NOT_FOUND&)
    {
        throw;
    }
    catch (const EX_UNKNOWN_STATUS&)
    {
        throw;
    }
    catch (const std::exception& e)
    {
        EX_INTERNAL_SERVER_ERROR ex;
        ex.what = e.what();
        throw ex;
    }
}

void BlockingImpl::unblockDomainsId(
        const IdlDomainIdList& _domain_list,
        const Nullable<std::string>& _new_owner,
        bool _remove_admin_c,
        const std::string& _reason,
        unsigned long long _log_req_id)
{
    Logging::Context ctx_server(create_ctx_name(this->get_server_name()));
    Logging::Context ctx_method("unblock-domains-id");

    administrative_unblock_domains(_domain_list,
            _new_owner,
            system_registrar_,
            _remove_admin_c,
            _reason,
            _log_req_id,
            false);
}

void BlockingImpl::blacklistAndDeleteDomainsId(
        const IdlDomainIdList& _domain_list,
        const Nullable<boost::gregorian::date>& _blacklist_to_date,
        const std::string& _reason,
        unsigned long long _log_req_id)
{
    Logging::Context ctx_server(create_ctx_name(this->get_server_name()));
    Logging::Context ctx_method("blacklist-and-delete-domains-id");

    EX_DOMAIN_ID_NOT_FOUND domain_id_not_found;
    try
    {
        boost::posix_time::ptime blacklist_to_limit;
        if (!_blacklist_to_date.isnull())
        {
            blacklist_to_limit = boost::posix_time::ptime(_blacklist_to_date.get_value(),
                    boost::posix_time::time_duration(12, 0, 0));
        }
        LibFred::OperationContextCreator ctx;
        for (IdlDomainIdList::const_iterator pDomainId = _domain_list.begin(); pDomainId != _domain_list.end(); ++pDomainId)
        {
            const LibFred::ObjectId object_id = *pDomainId;
            LibFred::CreateDomainNameBlacklistId create_domain_name_blacklist(object_id, _reason);
            if (!_blacklist_to_date.isnull())
            {
                create_domain_name_blacklist.set_valid_to(blacklist_to_limit);
            }
            try
            {
                create_domain_name_blacklist.exec(ctx);
                Database::Result object_handle_res = ctx.get_conn().exec_params(
                        "SELECT name "
                        "FROM object_registry "
                        "WHERE id=$1::bigint",
                        Database::query_param_list(object_id));
                if (object_handle_res.size() == 1)
                {
                    const std::string domain = static_cast<std::string>(object_handle_res[0][0]);
                    LibFred::DeleteDomainByFqdn(domain).exec(ctx);
                }
            }
            catch (const LibFred::CreateDomainNameBlacklistId::Exception& e)
            {
                if (e.is_set_object_id_not_found())
                {
                    domain_id_not_found.what.insert(e.get_object_id_not_found());
                }
                else
                {
                    throw;
                }
            }
            catch (const LibFred::DeleteDomainByFqdn::Exception& e)
            {
                if (e.is_set_unknown_domain_fqdn())
                {
                    domain_id_not_found.what.insert(object_id);
                }
                else
                {
                    throw;
                }
            }
        }
        if (!domain_id_not_found.what.empty())
        {
            throw domain_id_not_found;
        }
        ctx.commit_transaction();
    }
    catch (const EX_DOMAIN_ID_NOT_FOUND&)
    {
        throw;
    }
    catch (const std::exception& e)
    {
        EX_INTERNAL_SERVER_ERROR ex;
        ex.what = e.what();
        throw ex;
    }
}

void BlockingImpl::blacklistDomainsId(
        const IdlDomainIdList& _domain_list,
        const Nullable<boost::gregorian::date>& _blacklist_to_date,
        bool _with_delete,
        unsigned long long _log_req_id)
{
    Logging::Context ctx_server(create_ctx_name(this->get_server_name()));
    Logging::Context ctx_method("blacklist-domains-id");

    try
    {
        boost::posix_time::ptime blacklist_to_limit;
        if (!_blacklist_to_date.isnull())
        {
            blacklist_to_limit = boost::posix_time::ptime(_blacklist_to_date.get_value(),
                    boost::posix_time::time_duration(12, 0, 0));
        }
        LibFred::OperationContextCreator ctx;
        for (IdlDomainIdList::const_iterator pDomainId = _domain_list.begin(); pDomainId != _domain_list.end(); ++pDomainId)
        {
            const LibFred::ObjectId object_id = *pDomainId;
            LibFred::CreateDomainNameBlacklistId create_domain_name_blacklist(object_id, "blacklistDomainsId() call");
            if (!_blacklist_to_date.isnull())
            {
                create_domain_name_blacklist.set_valid_to(blacklist_to_limit);
            }
            create_domain_name_blacklist.exec(ctx);
        }
        ctx.commit_transaction();
    }
    catch (const EX_DOMAIN_ID_NOT_FOUND&)
    {
        throw;
    }
    catch (const std::exception& e)
    {
        EX_INTERNAL_SERVER_ERROR ex;
        ex.what = e.what();
        throw ex;
    }
}

} // namespace Fred::Backend::AdministrativeBlocking
} // namespace Fred::Backend
} // namespace Fred
