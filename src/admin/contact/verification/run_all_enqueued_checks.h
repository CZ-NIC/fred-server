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
 *  run all contact verification checks in queue
 */

#ifndef ADMIN_CONTACT_VERIFICATION_RUN_ALL_ENQUEUED_CHECKS_H_102876878
#define ADMIN_CONTACT_VERIFICATION_RUN_ALL_ENQUEUED_CHECKS_H_102876878

#include <vector>
#include <string>
#include <map>
#include <memory>

#include "src/admin/contact/verification/test_impl/test_interface.h"

namespace Admin {

    /**
     * Randomly (by happenstance, not even pseudo-random) runs all enqueued checks one by one
     *
     * @param _tests map of test objects denoted by their name
     * @return handles of executed (finalized) check ordered by execution (first in vector - first executed)
     */
    std::vector<std::string> run_all_enqueued_checks(
        const std::map<std::string, std::shared_ptr<Admin::ContactVerification::Test> >& _tests,
        Optional<unsigned long long> _logd_request_id = Optional<unsigned long long>());
}

#endif // #include guard end
