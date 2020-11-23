/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/public_request/confirmed_by.hh"
#include "src/backend/public_request/process/authinfo.hh"
#include "src/backend/public_request/process/block_unblock.hh"
#include "src/backend/public_request/process/personal_info.hh"
#include "src/backend/public_request/type/get_iface_of.hh"
#include "src/backend/public_request/type/public_request_authinfo.hh"
#include "src/backend/public_request/type/public_request_block_unblock.hh"
#include "src/backend/public_request/type/public_request_personal_info.hh"
#include "src/bin/cli/public_request_method.hh"
#include "libfred/opcontext.hh"
#include "libfred/public_request/public_request_on_status_action.hh"
#include "libfred/public_request/public_request_status.hh"
#include "src/deprecated/libfred/registry.hh"
#include "util/db/query_param.hh"

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>

#include <algorithm>
#include <functional>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace Admin {

namespace {

template <typename ...T>
std::unordered_map<std::string, const LibFred::PublicRequestTypeIface& (*)()> get_type_to_iface_mapping()
{
    std::unordered_map<std::string, const LibFred::PublicRequestTypeIface& (*)()> type_to_iface =
        {
            std::make_pair(Fred::Backend::PublicRequest::Type::get_iface_of<T>().get_public_request_type(),
                           Fred::Backend::PublicRequest::Type::get_iface_of<T>)...
        };
    return type_to_iface;
}

template <typename MK, typename MV, typename SK>
std::unordered_set<SK> insert_keys_from_umap_into_uset(const std::unordered_map<MK, MV>& m, std::unordered_set<SK>& s)
{
    for (const auto& key_value : m)
    {
        const bool took_place = s.insert(key_value.first).second;
        if (!took_place)
        {
            throw std::runtime_error("duplicate value detected");
        }
    }
    return s;
}

} // namespace Admin::{anonymous}

void PublicRequestProcedure::exec()
{
    namespace PublicRequestType = Fred::Backend::PublicRequest::Type;
    namespace PublicRequest = Fred::Backend::PublicRequest;

    const std::unordered_map<std::string, const LibFred::PublicRequestTypeIface& (*)()> type_authinfo_to_iface =
        get_type_to_iface_mapping<PublicRequestType::AuthinfoAutoRif,
                                  PublicRequestType::AuthinfoAuto,
                                  PublicRequestType::AuthinfoEmail,
                                  PublicRequestType::AuthinfoPost,
                                  PublicRequestType::AuthinfoGovernment>();

    const std::unordered_map<std::string, const LibFred::PublicRequestTypeIface& (*)()> type_personal_info_to_iface =
        get_type_to_iface_mapping<PublicRequestType::PersonalInfoAuto,
                                  PublicRequestType::PersonalInfoEmail,
                                  PublicRequestType::PersonalInfoPost,
                                  PublicRequestType::PersonalInfoGovernment>();

    const std::unordered_map<std::string, const LibFred::PublicRequestTypeIface& (*)()> type_block_unblock_to_iface =
        get_type_to_iface_mapping<PublicRequestType::BlockTransfer<PublicRequest::ConfirmedBy::email>,
                                  PublicRequestType::BlockTransfer<PublicRequest::ConfirmedBy::letter>,
                                  PublicRequestType::BlockTransfer<PublicRequest::ConfirmedBy::government>,
                                  PublicRequestType::BlockChanges<PublicRequest::ConfirmedBy::email>,
                                  PublicRequestType::BlockChanges<PublicRequest::ConfirmedBy::letter>,
                                  PublicRequestType::BlockChanges<PublicRequest::ConfirmedBy::government>,
                                  PublicRequestType::UnblockTransfer<PublicRequest::ConfirmedBy::email>,
                                  PublicRequestType::UnblockTransfer<PublicRequest::ConfirmedBy::letter>,
                                  PublicRequestType::UnblockTransfer<PublicRequest::ConfirmedBy::government>,
                                  PublicRequestType::UnblockChanges<PublicRequest::ConfirmedBy::email>,
                                  PublicRequestType::UnblockChanges<PublicRequest::ConfirmedBy::letter>,
                                  PublicRequestType::UnblockChanges<PublicRequest::ConfirmedBy::government>>();

    std::unordered_set<std::string> request_types_filter;
    {
        std::unordered_set<std::string> request_types_filter_default;
        insert_keys_from_umap_into_uset(type_authinfo_to_iface, request_types_filter_default);
        insert_keys_from_umap_into_uset(type_personal_info_to_iface, request_types_filter_default);
        insert_keys_from_umap_into_uset(type_block_unblock_to_iface, request_types_filter_default);

        for (const auto& argument: args_.types)
        {
            const auto itr = request_types_filter_default.find(argument);
            if (itr != request_types_filter_default.end())
            {
                request_types_filter.insert(*itr);
            }
            else
            {
                throw std::runtime_error("bad --types parameter: " + argument);
            }
        }
        if (request_types_filter.empty())
        {
            request_types_filter = std::move(request_types_filter_default);
        }
    }

    if (request_types_filter.empty())
    {
        return;
    }
    std::ostringstream condition_query_part;
    Database::query_param_list query_param_list;
    query_param_list(Conversion::Enums::to_db_handle(LibFred::PublicRequest::OnStatusAction::scheduled));
    condition_query_part << " AND eprt.name IN (";
    for (const auto& request_type : request_types_filter)
    {
        if (query_param_list.size() > 1)
        {
            condition_query_part << ",";
        }
        condition_query_part << "$" << query_param_list.size() + 1 << "::TEXT";
        query_param_list(request_type);
    }
    condition_query_part << ")";

    Database::Result dbres;
    {
        LibFred::OperationContextCreator ctx;
        dbres = ctx.get_conn().exec_params("SELECT pr.id, eprt.name, eprs.name "
                                           "FROM public_request pr "
                                           "JOIN enum_public_request_type eprt ON eprt.id=pr.request_type "
                                           "JOIN enum_public_request_status eprs ON eprs.id=pr.status "
                                           "WHERE pr.on_status_action=$1::enum_on_status_action_type" +
                                           condition_query_part.str(),
                                           query_param_list);
    }

    for (std::size_t i = 0; i < dbres.size(); ++i)
    {
        const auto request_id = static_cast<unsigned long long>(dbres[i][0]);
        const auto request_type = static_cast<std::string>(dbres[i][1]);
        const auto request_status =
            Conversion::Enums::from_db_handle<LibFred::PublicRequest::Status>(static_cast<std::string>(dbres[i][2]));
        try
        {
            namespace Fbpr = Fred::Backend::PublicRequest;
            if (request_status == LibFred::PublicRequest::Status::resolved)
            {
                const auto iface_personal_info_itr = type_personal_info_to_iface.find(request_type);
                if (iface_personal_info_itr != type_personal_info_to_iface.end())
                {
                    Fbpr::Process::process_public_request_personal_info_resolved(
                            request_id,
                            iface_personal_info_itr->second(),
                            mailer_manager_,
                            file_manager_client_);
                    continue;
                }
                const auto iface_authinfo_itr = type_authinfo_to_iface.find(request_type);
                if (iface_authinfo_itr != type_authinfo_to_iface.end())
                {
                    Fbpr::Process::process_public_request_authinfo_resolved(
                            request_id,
                            iface_authinfo_itr->second(),
                            mailer_manager_);
                    continue;
                }
                const auto iface_block_unblock_itr = type_block_unblock_to_iface.find(request_type);
                if (iface_block_unblock_itr != type_block_unblock_to_iface.end())
                {
                    Fbpr::Process::process_public_request_block_unblock_resolved(
                            request_id,
                            iface_block_unblock_itr->second(),
                            mailer_manager_);
                    continue;
                }
            }
        }
        catch (const std::exception& e)
        {
            LibFred::OperationContextCreator ctx;
            ctx.get_log().error(e.what());
        }
    }
}

} // namespace Admin
