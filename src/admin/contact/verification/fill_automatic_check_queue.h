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
 *  push back contact verification checks to queue up to it's at maximal length
 */

#ifndef ADMIN_CONTACT_VERIFICATION_FILL_QUEUE_H_12513457488
#define ADMIN_CONTACT_VERIFICATION_FILL_QUEUE_H_12513457488

#include <vector>
#include <string>

#include "fredlib/contact/verification/create_check.h"

namespace Admin {

    /**
     * fill contact verification queue by check with automatic testsuite
     *
     * @param _max_queue_length Will not add any check above this limit.
     */
    std::vector< boost::tuple<std::string, long long, long long> > fill_automatic_check_queue(
        unsigned _max_queue_length,
        Optional<long long> _logd_request_id = Optional<long long>());

}


#endif // #include guard end
