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
#include "util/idn_utils.h"

#include "src/admin/notification/notification.h"

namespace Admin {

    namespace Notification {

        namespace {

            bool domain_id_exists(const Fred::OperationContext &ctx, const unsigned long long domain_id) {
                const Database::Result result = ctx.get_conn().exec_params(
                    "SELECT 1 FROM domain WHERE id=$1::bigint FOR SHARE",
                    Database::query_param_list
                    (domain_id)
                );
                return result.size() ? true : false;
            }

            bool domain_is_expired(const Fred::OperationContext &ctx, const unsigned long long domain_id) {
                const Database::Result result = ctx.get_conn().exec_params(
                    "SELECT 1 FROM domain d "
                    "JOIN object_state os ON d.id = os.object_id "
                    "JOIN enum_object_states eos ON (os.state_id = eos.id AND eos.name = 'expired') "
                    "WHERE d.id = $1::bigint FOR SHARE",
                    Database::query_param_list
                    (domain_id)
                );
                return result.size() ? true : false;
            }

            static const int max_notification_email_length = 1024;

            bool email_valid(const std::string &email) {
                return ((Util::get_utf8_char_len(email) <= max_notification_email_length) && DjangoEmailFormat().check(email));
            }

            void cleanup_domain_emails(const Fred::OperationContext &ctx, const unsigned long long domain_id) {
                // clear all unnotified email records for the specified domain_id
                ctx.get_conn().exec_params(
                    "DELETE FROM notify_outzone_unguarded_domain_additional_email "
                    "WHERE domain_id = $1::bigint "
                      "AND state_id IS NULL",
                    Database::query_param_list
                    (domain_id)
                );
            }

            void add_domain_email(const Fred::OperationContext &ctx, const unsigned long long domain_id, const std::string &email) {
                // set specified email record for the specified domain_id
                ctx.get_conn().exec_params(
                    "INSERT INTO notify_outzone_unguarded_domain_additional_email "
                    "(crdate, state_id, domain_id, email) "
                    "VALUES ( "
                    "NOW(), "      // crdate
                    "NULL, "       // state_id (not yet available)
                    "$1::bigint, " // domain_id
                    "$2::varchar " // email
                    ")",
                    Database::query_param_list
                    (domain_id)
                    (email)
                 );
            }

            void set_domain_emails(
                    Fred::OperationContext &ctx,
                    const unsigned long long domain_id,
                    const std::set<std::string> &emails,
                    std::map<unsigned long long, std::set<std::string> > &invalid_domain_emails_map
                ) {
                cleanup_domain_emails(ctx, domain_id);

                // iterate through emails associated with the domain
                for(std::set<std::string>::const_iterator email_ptr = emails.begin(); email_ptr != emails.end(); ++email_ptr) {
                    add_domain_email(ctx, domain_id, *email_ptr);
                }
            }

        } // anonymous namespace


        void set_domain_outzone_unguarded_warning_emails(
            const std::map<unsigned long long, std::set<std::string> > &domain_emails_map
        ) {

            Fred::OperationContextCreator ctx;

            std::map<unsigned long long, std::set<std::string> > invalid_domain_emails_map;
            // check emails for validity
            for(std::map<unsigned long long, std::set<std::string> >::const_iterator i = domain_emails_map.begin(); i != domain_emails_map.end(); ++i) {
                for(std::set<std::string>::const_iterator email_ptr = i->second.begin(); email_ptr != i->second.end(); ++email_ptr) {
                    if(!email_valid(*email_ptr)) {
                        ctx.get_log().warning(boost::format("invalid email address %1% for domain id %1%") % (*email_ptr) % i->first); // ?
                        invalid_domain_emails_map[i->first].insert(*email_ptr);
                    }
                }
            }
            if (!invalid_domain_emails_map.empty()) {
                ctx.get_log().warning("invalid emails or domain ids");
                throw DomainEmailValidationError(invalid_domain_emails_map);
            }

            try {

                // set emails
                for(std::map<unsigned long long, std::set<std::string> >::const_iterator i = domain_emails_map.begin(); i != domain_emails_map.end(); ++i) {
                    if(domain_id_exists(ctx, i->first)) {
                        if(domain_is_expired(ctx, i->first)) {
                            set_domain_emails(ctx, i->first, i->second, invalid_domain_emails_map);
                        }
                        else {
                            ctx.get_log().warning(boost::format("active expired domain with id %1% not found, either it is no more active or expired or the id is incorrect") % i->first);
                        }
                    }
                    else {
                        ctx.get_log().warning(boost::format("domain with id %1% not found") % i->first);
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
