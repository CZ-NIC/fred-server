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
 *  @file
 *  declaration for Notification Corba conversion
 */

#ifndef CORBA_NOTIFICATION_CORBA_CONVERSION_H_
#define CORBA_NOTIFICATION_CORBA_CONVERSION_H_

#include "util/corba_conversion.h"

#include "src/corba/Notification.hh"

#include "src/admin/notification/notification.h"

namespace CorbaConversion {

/**
 * \brief convert Corba DomainEmailSeq to C++ map of sets of emails (domain_emails_map)
 *
 * \param domain_email_seq   sequence of {domain_id, email}
 * \param domain_emails_map  emails by domain_id
 */
void unwrap_notification_emails(const Registry::Notification::DomainEmailSeq &src, std::map<unsigned long long, std::set<std::string> > &dst);

/**
 * \brief  convert implementation type to interface type
 *
 * \param domain_emails_map  emails by domain_id
 * \param domain_email_seq   sequence of {domain_id, email}
 */
void wrap_notification_emails(const std::map<unsigned long long, std::set<std::string> > &domain_emails_map, Registry::Notification::DomainEmailSeq_var &domain_email_seq);

/**
 * \brief convert implementation exception to interface exception
 *
 * \param src  implementation exception
 */
void raise_DOMAIN_EMAIL_VALIDATION_ERROR(const Admin::Notification::DomainEmailValidationError &src);

} // namespace CorbaConversion

#endif
