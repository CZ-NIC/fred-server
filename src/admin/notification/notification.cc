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

#include "util/db/query_param.h"

#include "src/admin/notification/notification.h"

namespace Admin {

    namespace Notification {

        void notify_outzone_unguarded_domain_email_list(
            const std::map<unsigned long long, std::set<std::string> > &domain_emails_map
        ) {

            Fred::OperationContextCreator ctx;

            std::map<unsigned long long, std::set<std::string> > invalid_domain_emails_map;

            for(std::map<unsigned long long, std::set<std::string> >::const_iterator i = domain_emails_map.begin(); i != domain_emails_map.end(); ++i) {
                for(std::set<std::string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {

                    bool invalid_email = false;
                    // TODO email length check in MOUJEID:
                    //enum { MAX_MOJEID_EMAIL_LENGTH = 200 };
                    //((Util::get_utf8_char_len(email) <= MAX_MOJEID_EMAIL_LENGTH)

                    if (j->empty() || !DjangoEmailFormat().check(*j)) {
                        invalid_email = true;
                    }

                    bool invalid_id = false;
                    const Database::Result r = ctx.get_conn().exec_params(
                        "SELECT from domain_history WHERE id=$1::bigint",
                        Database::query_param_list
                        (i->first) // domain_id
                    );
                    if (r.size() == 0) {
                        invalid_id = true;
                    }

                    if(invalid_email || invalid_id) {
                        invalid_domain_emails_map[i->first].insert(*j);
                    }
                }
            }

            if (!invalid_domain_emails_map.empty()) {
                ctx.get_log().warning("invalid emails or domain ids");
                throw DomainEmailValidationError(invalid_domain_emails_map);
            }

            try {
                for(std::map<unsigned long long, std::set<std::string> >::const_iterator i = domain_emails_map.begin(); i != domain_emails_map.end(); ++i) {
                    // clear unnotified email records for the specified domain_id
                    ctx.get_conn().exec_params(
                        "DELETE FROM notify_outzone_unguarded_domain_additional_email "
                        "WHERE domain_id = $1::bigint "
                          "AND state_id IS NULL",
                        Database::query_param_list
                        (i->first)  // domain_id
                    );
                }
                for(std::map<unsigned long long, std::set<std::string> >::const_iterator i = domain_emails_map.begin(); i != domain_emails_map.end(); ++i) {
                    for(std::set<std::string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {

                        // set specified email records for the specified domain_id
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
                            (i->first)  // domain_id
                            (*j)         // email
                        );
                    }
                }
            } catch (const std::exception &e) {
                ctx.get_log().error(e.what());
                throw InternalError();
            } catch (...) {
                ctx.get_log().error("unknown exception");
                throw InternalError();
            }

            ctx.commit_transaction();

            return;
        };

    }

}

/* vim: set et sw=4 : */
