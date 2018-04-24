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
#include "src/backend/public_request/process_public_requests.hh"
#include "src/util/db/query_param.hh"
#include "src/bin/cli/public_request_method.hh"

namespace Admin {

void PublicRequestProcedure::exec()
{
    LibFred::OperationContextCreator ctx;
    const Database::Result dbres =
        ctx.get_conn().exec_params("SELECT id, request_type FROM public_request WHERE on_status_action=$1::enum_on_status_action_type",
                                    Database::query_param_list
                                    (Conversion::Enums::to_db_handle(LibFred::PublicRequest::OnStatusAction::scheduled)));
    for (std::size_t i = 0; i < dbres.size(); ++i)
    {
        const auto request_id = static_cast<unsigned long long>(dbres[i][0]);
        const auto request_type = static_cast<unsigned long long>(dbres[i][1]); // enum or sql query?
        try
        {
            if (request_type == 22 || request_type == 23 || request_type == 24)
            {
                Fred::Backend::PublicRequest::process_public_request_personal_info_answered(
                        request_id,
                        ctx,
                        mailer_manager,
                        file_manager_client);
            }
            else
            {
                Fred::Backend::PublicRequest::process_public_request_nop(
                        request_id,
                        ctx,
                        mailer_manager,
                        file_manager_client);
            }
        }
        catch(const std::exception& e)
        {
            ctx.get_log().error(e.what());
        }
    }
}

} // namespace Admin
