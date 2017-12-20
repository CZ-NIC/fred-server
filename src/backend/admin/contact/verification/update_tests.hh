/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  @file
 *  update tests of a check
 */

#ifndef ADMIN_CONTACT_VERIFICATION_UPDATE_TESTS_H_1001456454560155
#define ADMIN_CONTACT_VERIFICATION_UPDATE_TESTS_H_1001456454560155

#include "src/backend/admin/contact/verification/exceptions.hh"

#include "src/util/db/nullable.hh"
#include "src/util/optional_value.hh"
#include "src/libfred/opcontext.hh"

#include <vector>
#include <utility>
#include <string>

namespace Admin {
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
        unsigned long long                      _logd_request_id);

}

#endif // #include guard end
