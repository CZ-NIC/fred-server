/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#include "src/libfred/registry.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"
#include "src/backend/public_request/process_public_request_authinfo.hh"
#include "src/backend/public_request/process_public_request_block_unblock.hh"
#include "src/backend/public_request/process_public_request_personal_info.hh"
#include "src/util/db/query_param.hh"
#include "src/bin/cli/public_request_method.hh"
#include "src/backend/public_request/type/public_request_authinfo.hh"
#include "src/backend/public_request/type/public_request_block_unblock.hh"
#include "src/backend/public_request/type/public_request_personal_info.hh"
#include "src/backend/public_request/type/get_iface_of.hh"
#include "src/libfred/public_request/public_request_status.hh"
#include "src/libfred/opcontext.hh"

#include <unordered_map>
#include <functional>
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

template <typename ...T>
std::set<std::string> get_request_type_filter()
{
    std::set<std::string> request_types_filter =
        {
            Fred::Backend::PublicRequest::Type::get_iface_of<T>().get_public_request_type()...
        };
    return request_types_filter;
}

} // namespace Admin::{anonymous}

void PublicRequestProcedure::exec()
{
    namespace PublicRequestType = Fred::Backend::PublicRequest::Type;
    namespace PublicRequest = Fred::Backend::PublicRequest;

    std::set<std::string> request_types_filter;
    {
        std::set<std::string> request_types_filter_default =
            get_request_type_filter<PublicRequestType::AuthinfoAuto,
                                    PublicRequestType::AuthinfoEmail,
                                    PublicRequestType::AuthinfoPost,
                                    PublicRequestType::AuthinfoGovernment,
                                    PublicRequestType::PersonalInfoAuto,
                                    PublicRequestType::PersonalInfoEmail,
                                    PublicRequestType::PersonalInfoPost,
                                    PublicRequestType::PersonalInfoGovernment,
                                    PublicRequestType::BlockTransfer<PublicRequest::PublicRequestImpl::ConfirmedBy::email>,
                                    PublicRequestType::BlockTransfer<PublicRequest::PublicRequestImpl::ConfirmedBy::letter>,
                                    PublicRequestType::BlockTransfer<PublicRequest::PublicRequestImpl::ConfirmedBy::government>,
                                    PublicRequestType::BlockChanges<PublicRequest::PublicRequestImpl::ConfirmedBy::email>,
                                    PublicRequestType::BlockChanges<PublicRequest::PublicRequestImpl::ConfirmedBy::letter>,
                                    PublicRequestType::BlockChanges<PublicRequest::PublicRequestImpl::ConfirmedBy::government>,
                                    PublicRequestType::UnblockTransfer<PublicRequest::PublicRequestImpl::ConfirmedBy::email>,
                                    PublicRequestType::UnblockTransfer<PublicRequest::PublicRequestImpl::ConfirmedBy::letter>,
                                    PublicRequestType::UnblockTransfer<PublicRequest::PublicRequestImpl::ConfirmedBy::government>,
                                    PublicRequestType::UnblockChanges<PublicRequest::PublicRequestImpl::ConfirmedBy::email>,
                                    PublicRequestType::UnblockChanges<PublicRequest::PublicRequestImpl::ConfirmedBy::letter>,
                                    PublicRequestType::UnblockChanges<PublicRequest::PublicRequestImpl::ConfirmedBy::government>>();
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

    const std::unordered_map<std::string, const LibFred::PublicRequestTypeIface& (*)()> type_to_iface =
        get_type_to_iface_mapping<PublicRequestType::AuthinfoAutoRif,
                                  PublicRequestType::AuthinfoAuto,
                                  PublicRequestType::AuthinfoEmail,
                                  PublicRequestType::AuthinfoPost,
                                  PublicRequestType::AuthinfoGovernment,
                                  PublicRequestType::PersonalInfoAuto,
                                  PublicRequestType::PersonalInfoEmail,
                                  PublicRequestType::PersonalInfoPost,
                                  PublicRequestType::PersonalInfoGovernment,
                                  PublicRequestType::BlockTransfer<PublicRequest::PublicRequestImpl::ConfirmedBy::email>,
                                  PublicRequestType::BlockTransfer<PublicRequest::PublicRequestImpl::ConfirmedBy::letter>,
                                  PublicRequestType::BlockTransfer<PublicRequest::PublicRequestImpl::ConfirmedBy::government>,
                                  PublicRequestType::BlockChanges<PublicRequest::PublicRequestImpl::ConfirmedBy::email>,
                                  PublicRequestType::BlockChanges<PublicRequest::PublicRequestImpl::ConfirmedBy::letter>,
                                  PublicRequestType::BlockChanges<PublicRequest::PublicRequestImpl::ConfirmedBy::government>,
                                  PublicRequestType::UnblockTransfer<PublicRequest::PublicRequestImpl::ConfirmedBy::email>,
                                  PublicRequestType::UnblockTransfer<PublicRequest::PublicRequestImpl::ConfirmedBy::letter>,
                                  PublicRequestType::UnblockTransfer<PublicRequest::PublicRequestImpl::ConfirmedBy::government>,
                                  PublicRequestType::UnblockChanges<PublicRequest::PublicRequestImpl::ConfirmedBy::email>,
                                  PublicRequestType::UnblockChanges<PublicRequest::PublicRequestImpl::ConfirmedBy::letter>,
                                  PublicRequestType::UnblockChanges<PublicRequest::PublicRequestImpl::ConfirmedBy::government>>();
    for (std::size_t i = 0; i < dbres.size(); ++i)
    {
        const auto request_id = static_cast<unsigned long long>(dbres[i][0]);
        const auto request_type = static_cast<std::string>(dbres[i][1]);
        const auto request_status =
            Conversion::Enums::from_db_handle<LibFred::PublicRequest::Status>(static_cast<std::string>(dbres[i][2]));
        try
        {
            if (request_status == LibFred::PublicRequest::Status::resolved)
            {
                if (request_type == "personalinfo_auto_pif" ||
                    request_type == "personalinfo_email_pif" ||
                    request_type == "personalinfo_post_pif" ||
                    request_type == "personalinfo_government_pif")
                {
                    Fred::Backend::PublicRequest::process_public_request_personal_info_resolved(
                            request_id,
                            type_to_iface.at(request_type)(),
                            mailer_manager_,
                            file_manager_client_);
                }
                else if (request_type == "authinfo_auto_rif" ||
                         request_type == "authinfo_auto_pif" ||
                         request_type == "authinfo_email_pif" ||
                         request_type == "authinfo_post_pif" ||
                         request_type == "authinfo_government_pif")
                {
                    Fred::Backend::PublicRequest::process_public_request_auth_info_resolved(
                            request_id,
                            type_to_iface.at(request_type)(),
                            mailer_manager_);
                }
                else if (request_type == "block_transfer_email_pif" ||
                         request_type == "block_transfer_post_pif" ||
                         request_type == "block_transfer_government_pif" ||
                         request_type == "block_changes_email_pif" ||
                         request_type == "block_changes_post_pif" ||
                         request_type == "block_changes_government_pif" ||
                         request_type == "unblock_transfer_email_pif" ||
                         request_type == "unblock_transfer_post_pif" ||
                         request_type == "unblock_transfer_government_pif" ||
                         request_type == "unblock_changes_email_pif" ||
                         request_type == "unblock_changes_post_pif" ||
                         request_type == "unblock_changes_government_pif")
                {
                    Fred::Backend::PublicRequest::process_public_request_block_unblock_resolved(
                            request_id,
                            type_to_iface.at(request_type)());
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
