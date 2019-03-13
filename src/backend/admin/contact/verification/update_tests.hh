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
/**
 *  @file
 *  update tests of a check
 */

#ifndef UPDATE_TESTS_HH_F15339FCB033419B9AFE5ECD6E2E41DB
#define UPDATE_TESTS_HH_F15339FCB033419B9AFE5ECD6E2E41DB

#include "src/backend/admin/contact/verification/exceptions.hh"

#include "libfred/opcontext.hh"
#include "util/db/nullable.hh"
#include "util/optional_value.hh"

#include <string>
#include <utility>
#include <vector>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

using std::vector;
using std::pair;
using std::string;

/**
 * setting new statuses to tests of check
 *
 * @param _test_statuses are test_handle, test_status_handle pairs
 *
 * @throws Admin::ExceptionUnknownCheckHandle
 * @throws Admin::ExceptionUnknownTestHandle
 * @throws Admin::ExceptionUnknownCheckTestPair
 * @throws Admin::ExceptionUnknownTestStatusHandle
 * @throws Admin::ExceptionCheckNotUpdateable
 */

void update_tests(
        LibFred::OperationContext&                 _ctx,
        const uuid&                             _check_handle,
        const vector<pair<string, string> >&    _test_statuses,
        unsigned long long _logd_request_id);

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif
