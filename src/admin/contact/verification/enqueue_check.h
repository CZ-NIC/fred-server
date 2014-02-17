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
 *  enqueue specified check (and invalidates checks obsoleted by it)
 */

#ifndef ADMIN_CONTACT_VERIFICATION_ENQUEUE_CHECK_H_32421015464
#define ADMIN_CONTACT_VERIFICATION_ENQUEUE_CHECK_H_32421015464

#include "src/admin/contact/verification/exceptions.h"

#include "src/fredlib/contact/verification/create_check.h"

#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"

namespace Admin {

    /**
     * @throws Admin::ExceptionUnknownContactId
     * @throws Admin::ExceptionUnknownTestsuiteHandle
     */
    std::string enqueue_check(
        Fred::OperationContext&         _ctx,
        unsigned long long              _contact_id,
        const std::string&              _testsuite_handle,
        Optional<unsigned long long>    _logd_request_id = Optional<unsigned long long>());
}


#endif // #include guard end
