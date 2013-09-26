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

#include "admin/contact/verification/test_impl/test_interface.h"

namespace Admin {
    struct ExceptionNoEnqueuedChecksAvailable : public std::exception {};
    struct ExceptionCheckTestsuiteFullyCreated : public std::exception {};

    /**
     * Randomly (by happenstance, not even pseudo-random) selects some enqueued check and execute it (by running it's tests).
     *
     * @param _tests map of test objects denoted by their name
     * @return handle of selected and executed check
     */
    std::string run_first_enqueued_check(const std::map<std::string, boost::shared_ptr<Admin::ContactVerificationTest> >& _tests);

}


#endif // #include guard end
