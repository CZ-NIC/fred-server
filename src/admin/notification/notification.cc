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

#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object/object_state.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/contact_verification/django_email_format.h"

#include "util/db/query_param.h"
#include "util/idn_utils.h"

#include "src/admin/notification/notification.h"

namespace Admin {
namespace Notification {

namespace {

bool domain_id_exists(Fred::OperationContext &ctx, const unsigned long long domain_id) {
    try {
        Fred::InfoDomainById(domain_id).exec(ctx);
    }
    catch (const Fred::InfoDomainById::Exception &e) {
        if (e.is_set_unknown_object_id()) {
            return false;
        }
        throw;
    }
    return true;
}

bool domain_is_expired(Fred::OperationContext &ctx, const unsigned long long domain_id) {
    const std::vector<Fred::ObjectStateData> states_data = Fred::GetObjectStates(domain_id).exec(ctx);
    for (
        std::vector<Fred::ObjectStateData>::const_iterator data_ptr = states_data.begin();
        data_ptr != states_data.end();
        ++data_ptr)
    {
        if (Conversion::Enums::from_db_handle<Fred::Object_State>(data_ptr->state_name) == Fred::Object_State::expired) {
            return true;
        }
    }
    return false;
}

static const int max_notification_email_length = 1024;

bool email_valid(const std::string &email) {
    return (Util::get_utf8_char_len(email) <= max_notification_email_length) && DjangoEmailFormat().check(email);
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
    if (email.empty()) {
        return;
    }
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
        const std::set<std::string> &emails
    ) {
    cleanup_domain_emails(ctx, domain_id);

    for (std::set<std::string>::const_iterator email_ptr = emails.begin(); email_ptr != emails.end(); ++email_ptr) {
        if (!email_ptr->empty()) {
            add_domain_email(ctx, domain_id, *email_ptr);
        }
    }
}

} // anonymous namespace


void set_domain_outzone_unguarded_warning_emails(
    const std::map<unsigned long long, std::set<std::string> > &domain_emails_map
) {

    Fred::OperationContextCreator ctx;

    std::map<unsigned long long, std::set<std::string> > domain_invalid_emails_map;

    try {

        // check emails for validity
        for (
            std::map<unsigned long long, std::set<std::string> >::const_iterator domain_emails_map_iter = domain_emails_map.begin();
            domain_emails_map_iter != domain_emails_map.end();
            ++domain_emails_map_iter)
        {
            for (
                std::set<std::string>::const_iterator email_ptr = domain_emails_map_iter->second.begin();
                email_ptr != domain_emails_map_iter->second.end();
                ++email_ptr)
            {
                if (!email_ptr->empty() && !email_valid(*email_ptr)) {
                    ctx.get_log().warning(boost::format("invalid email address for domain id %1%") % domain_emails_map_iter->first);
                    domain_invalid_emails_map[domain_emails_map_iter->first].insert(*email_ptr);
                }
            }
        }
        if (!domain_invalid_emails_map.empty()) {
            throw DomainEmailValidationError(domain_invalid_emails_map);
        }

        // set emails
        for (
            std::map<unsigned long long, std::set<std::string> >::const_iterator domain_emails_map_iter = domain_emails_map.begin();
            domain_emails_map_iter != domain_emails_map.end();
            ++domain_emails_map_iter)
        {
            if (domain_id_exists(ctx, domain_emails_map_iter->first)) {
                if (domain_is_expired(ctx, domain_emails_map_iter->first)) {
                    set_domain_emails(ctx, domain_emails_map_iter->first, domain_emails_map_iter->second);
                }
                else {
                    ctx.get_log().warning(boost::format("active expired domain with id %1% not found, either it is no more active or expired or the id is incorrect") % domain_emails_map_iter->first);
                }
            }
            else {
                ctx.get_log().warning(boost::format("domain with id %1% not found") % domain_emails_map_iter->first);
            }
        }

    } catch (const DomainEmailValidationError& e) {
        ctx.get_log().warning(e.what());
        throw;
    } catch (const std::exception &e) {
        ctx.get_log().error(e.what());
        throw InternalError();
    } catch (...) {
        ctx.get_log().error("unknown exception");
        throw InternalError();
    }

    ctx.commit_transaction();
}

} // namespace Notification
} // namespace Admin
