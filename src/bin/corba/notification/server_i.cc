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

#include <stdexcept>

#include "src/bin/corba/util/corba_conversions_nullable_types.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"

#include "src/deprecated/libfred/notify.hh"

#include "src/backend/admin/notification/notification.hh"

#include "src/bin/corba/Notification.hh"

#include "src/bin/corba/notification/notification_corba_conversion.hh"

#include "src/bin/corba/notification/server_i.hh"

namespace Registry {
namespace Notification {

/* Create string for logging context
 * @param server_name  server name
 * @return  "server_name-<call_id>"
 */
static const std::string create_ctx_name(const std::string &server_name)
{
    return boost::str(boost::format("%1%-<%2%>")% server_name % Random::integer(0, 10000));
}

Server_i::Server_i(const std::string &server_name)
    : server_name(server_name)
{
    Logging::Context lctx_server(server_name);
    Logging::Context lctx("init");

}

std::string Server_i::get_server_name()
{
    return server_name;
}

void Server_i::set_domain_outzone_unguarded_warning_emails(const DomainEmailSeq &domain_email_seq)
{
    Logging::Context lctx_server(create_ctx_name(get_server_name()));
    Logging::Context lctx("set-domain-outzone-unguarded-warning-emails");

    try {
        std::map<unsigned long long, std::set<std::string> > domain_emails_map;
        CorbaConversion::unwrap_DomainEmailSeq(domain_email_seq, domain_emails_map);

        Admin::Notification::set_domain_outzone_unguarded_warning_emails(domain_emails_map);

    } catch (const Admin::Notification::DomainEmailValidationError &e) {
        CorbaConversion::raise_DOMAIN_EMAIL_VALIDATION_ERROR(e);
    } catch (...) {
        throw INTERNAL_SERVER_ERROR();
    }

}

} // namespace Notification
} // namespace Registry
