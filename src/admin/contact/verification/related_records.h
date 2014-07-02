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

#include <vector>
#include <string>
#include <set>
#include <boost/tuple/tuple.hpp>

#include "src/fredlib/opcontext.h"

namespace Admin {
    using std::vector;
    using std::string;

    /**
     * Stores the relation between check and mail.
     * Mail itself has to be already created otherwise postgres exception is thrown.
     */
    void add_related_mail(
        Fred::OperationContext&             _ctx,
        const uuid&                         _check_handle,
        const std::set<unsigned long long>& _mail_archive_ids
    );

    /**
     * Stores the relation between check and message.
     * Message itself has to be already created otherwise postgres exception is thrown.
     */
    void add_related_messages(
        Fred::OperationContext&             _ctx,
        const uuid&                         _check_handle,
        const std::set<unsigned long long>& _message_archive_ids);

    /**
     * Stores the relation between check and object state request.
     * Object state request itself has to be already created otherwise postgres exception is thrown.
     */
    void add_related_object_state_requests(
        Fred::OperationContext&             _ctx,
        const uuid&                         _check_handle,
        const std::set<unsigned long long>& _request_ids);


    struct related_message {
        unsigned long long                  id;
        string                              comm_type;
        string                              content_type;
        boost::posix_time::ptime            created;
        Nullable<boost::posix_time::ptime>  update;
        unsigned long                       status_id;
        string                              status_name;

        related_message(
            unsigned long long                          _id,
            const string&                               _comm_type,
            const string&                               _content_type,
            const boost::posix_time::ptime&             _created,
            const Nullable<boost::posix_time::ptime>&   _update,
            unsigned long                               _status_id,
            const string&                               _status_name
        ) :
            id(_id),
            comm_type(_comm_type),
            content_type(_content_type),
            created(_created),
            update(_update),
            status_id(_status_id),
            status_name(_status_name)
        { }
    };

    /**
     * @returns related messages info
     */
    vector<related_message> get_related_messages(
        Fred::OperationContext& _ctx,
        unsigned long long      _contact_id,
        const std::string&      _output_timezone = "Europe/Prague");
}


#endif // #include guard end
