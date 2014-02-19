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
 *  run first contact verification check in queue
 */

#ifndef ADMIN_CONTACT_VERIFICATION_RUN_FIRST_ENQUEUED_CHECK_H_102876878
#define ADMIN_CONTACT_VERIFICATION_RUN_FIRST_ENQUEUED_CHECK_H_102876878

#include <vector>
#include <string>
#include <map>

#include <boost/shared_ptr.hpp>

#include "util/optional_value.h"
#include "src/fredlib/opcontext.h"

#include "src/admin/contact/verification/test_impl/test_interface.h"
#include "src/fredlib/contact/verification/enum_testsuite_handle.h"

namespace Admin {

    void preprocess_automatic_check(
        Fred::OperationContext& _ctx,
        const std::string& _check_handle);

    void preprocess_manual_check(
        Fred::OperationContext& _ctx,
        const std::string& _check_handle);

    /**
     * Randomly (by happenstance, not even pseudo-random) selects some enqueued check and execute it (by running it's tests).
     *
     * @param _tests map of test objects denoted by their name
     * @return handle of selected and executed check
     */
    Optional<std::string> run_first_enqueued_check(
        const std::map<
            std::string,
            boost::shared_ptr<Admin::ContactVerification::Test>
        >&                              _tests,
        Optional<unsigned long long>    _logd_request_id);

}


#endif // #include guard end
