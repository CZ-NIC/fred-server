/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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
 *  @file notification.cc
 *  corba server implementation of registry notification
 */

#include "src/fredlib/opcontext.h"

#include "src/admin/notification/notification.h"

namespace Admin {

    namespace Notification {

        /**
         * Import list of additional emails used to notify \c outboundUnguardedWarning domain state
         *
         * @param list of pairs <domain_id, email>
         *
         * @throw INTERNAL_ERROR
         * @throw INVALID_VALUE
         */
        void notify_outzoneunguarded_domain_email_list(
            Fred::OperationContext &ctx,
            const std::vector<std::pair<unsigned long long, std::string> > &domain_email_list
        ) {

            unsigned long long index = 0;
            try {
                // TODO partial import?
                //ctx.get_conn().exec("RELEASE SAVEPOINT outzoneunguarded_domain_email_savepoint");
                //ctx.get_conn().exec("SAVEPOINT outzoneunguarded_domain_email_savepoint");
                for(std::vector<std::pair<unsigned long long, std::string> >::const_iterator it = domain_email_list.begin();
                    it != domain_email_list.end();
                    ++it, ++index
                ) {
                    ctx.get_conn().exec_params(
                        "INSERT INTO notify_outzoneunguarded_domain_additional_email "
                           "(id, crdate, state_id, email) "
                           "VALUES( "
                               "0, "          // id
                               "NOW(), "      // crdate
                               "(SELECT id FROM object_state WHERE object_id=$1::bigint and state_id = 15 and valid_to IS NULL), " // domain_id -> state_id; 15: outzone
                               "$2::varchar " // email
                           ")",
                        Database::query_param_list
                        (it->first)
                        (it->second)
                        );
                }
            } catch(...) {
                //try {
                //    //ctx.get_conn().exec("ROLLBACK TO outzoneunguarded_domain_email_savepoint");
                //} catch(...) {
                //    // exception consumed
                //}
                throw VALUE_ERROR(index);
            }
        };

    }

}

/* vim: set et sw=4 : */
