/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/admin/contact/verification/list_active_checks.hh"

#include "util/log/context.hh"

#include <boost/assign/list_of.hpp>

#include <vector>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

std::vector<LibFred::ListChecksItem> list_active_checks(const Optional<std::string>& _testsuite_handle, const std::string& _output_timezone)
{
    Logging::Context log("list_active_checks");

    std::vector<LibFred::ListChecksItem> result;

    std::vector<std::string> awaiting_statuses = LibFred::ContactCheckStatus::get_not_yet_resolved();

    LibFred::OperationContextCreator ctx;
    std::vector<LibFred::ListChecksItem> temp_result;
    for (std::vector<std::string>::const_iterator it = awaiting_statuses.begin();
         it != awaiting_statuses.end();
         ++it)
    {
        temp_result = LibFred::ListContactChecks()
                      .set_status_handle(*it)
                      .exec(ctx, _output_timezone);

        result.reserve(result.size() + temp_result.size());
        result.insert(
                result.end(),
                temp_result.begin(),
                temp_result.end());
    }

    return result;
}

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
