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
 *  utilities for records related to admin contact verification data
 */

#ifndef ADMIN_CONTACT_VERIFICATION_RELATED_RECORDS_H_56564542442
#define ADMIN_CONTACT_VERIFICATION_RELATED_RECORDS_H_56564542442

#include <set>

#include "src/fredlib/opcontext.h"

namespace Admin {

    /**
     * Stores the relation between check and mail.
     * Mail itself has to be already created otherwise postgres exception is thrown.
     */
    void add_related_mail(
        Fred::OperationContext&             _ctx,
        const std::string&                  _check_handle,
        const std::set<unsigned long long>& _mail_archive_ids
    );

    /**
     * Stores the relation between check and message.
     * Message itself has to be already created otherwise postgres exception is thrown.
     */
    void add_related_messages(
        Fred::OperationContext&             _ctx,
        const std::string&                  _check_handle,
        const std::set<unsigned long long>& _message_archive_ids);

    /**
     * Stores the relation between check and object state request.
     * Object state request itself has to be already created otherwise postgres exception is thrown.
     */
    void add_related_object_state_requests(
        Fred::OperationContext&             _ctx,
        const std::string&                  _check_handle,
        const std::set<unsigned long long>& _request_ids);
}


#endif // #include guard end
