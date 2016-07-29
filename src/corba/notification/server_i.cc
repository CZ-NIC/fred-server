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

#include "src/corba/util/corba_conversions_nullable_types.h"
#include "src/corba/util/corba_conversions_string.h"

#include "src/fredlib/notify.h"

#include "src/admin/notification/notification.h"

#include "src/corba/Notification.hh"

#include "src/corba/notification/notification_corba_conversion.h"

#include "src/corba/notification/server_i.h"

namespace Registry {
namespace Notification {

void Server_i::set_domain_outzone_unguarded_warning_emails(const DomainEmailSeq &domain_email_seq) {

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
