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
#include "src/fredlib/contact_verification/django_email_format.h"

#include "src/admin/notification/notification.h"

namespace Admin {

    namespace Notification {

        void notify_outzone_unguarded_domain_email_list(
            const std::vector<std::pair<unsigned long long, std::string> > &domain_email_list
        ) {

            Fred::OperationContextCreator ctx;

            unsigned long index = 0;
            std::vector<std::pair<unsigned long long, std::string> > invalid_domain_email_list;

            for (std::vector<std::pair<unsigned long long, std::string> >::const_iterator it = domain_email_list.begin();
                it != domain_email_list.end();
                ++it, ++index
            ) {
                // TODO email length check in MOUJEID:
                //enum { MAX_MOJEID_EMAIL_LENGTH = 200 };
                //((Util::get_utf8_char_len(email) <= MAX_MOJEID_EMAIL_LENGTH)

                if (it->second.empty() || !DjangoEmailFormat().check(it->second)) {
                    invalid_domain_email_list.push_back(*it);
                }

                const Database::Result r = ctx.get_conn().exec_params(
                    "SELECT from domain_history WHERE id=$1::bigint",
                    Database::query_param_list
                    (it->first)
                );
                if (r.size() == 0) {
                    invalid_domain_email_list.push_back(*it);
                }
                // FIXME push_back(*it) just once
            }

            if (invalid_domain_email_list.size()) {
                ctx.get_log().warning("invalid emails or domain ids");
                throw DOMAIN_EMAIL_VALIDATION_ERROR(invalid_domain_email_list);
            }

            try {
                for (std::vector<std::pair<unsigned long long, std::string> >::const_iterator it = domain_email_list.begin();
                    it != domain_email_list.end();
                    ++it, ++index
                ) {
                    ctx.get_conn().exec_params(
                        "INSERT INTO notify_outzone_unguarded_domain_additional_email "
                           "(crdate, state_id, domain_id, email) "
                           "VALUES ( "
                               "NOW(), "      // crdate
                               "NULL, "       // state_id, N/A yet
                               "$1::bigint, " // domain_id
                               "$2::varchar " // email
                           ")",
                        Database::query_param_list
                        (it->first)  // domain_id
                        (it->second) // email
                    );
                }
            } catch (const std::runtime_error &e) {
                ctx.get_log().error(e.what());
                throw INTERNAL_ERROR();
            } catch (const std::exception &e) {
                ctx.get_log().error(e.what());
                throw INTERNAL_ERROR();
            } catch (...) {
                ctx.get_log().error("unknown exception");
                throw INTERNAL_ERROR();
            }

            ctx.commit_transaction();

            return;
        };

    }

}

/* vim: set et sw=4 : */
