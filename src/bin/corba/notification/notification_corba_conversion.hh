/*
 * Copyright (C) 2016-2020  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file
 *  declaration for Notification Corba conversion
 */

#ifndef NOTIFICATION_CORBA_CONVERSION_HH_591B64BF8448432E83DA24E6AA6AA048
#define NOTIFICATION_CORBA_CONVERSION_HH_591B64BF8448432E83DA24E6AA6AA048

#include "src/util/corba_conversion.hh"

#include "corba/Notification.hh"

#include "src/backend/admin/notification/notification.hh"

namespace CorbaConversion {

/**
 * \brief convert Corba DomainEmailSeq to STL map of domain_ids to sets of emails (domain_emails_map)
 *
 * \param domain_email_seq   sequence of {domain_id, email}
 * \param domain_emails_map  emails by domain_id
 */
void unwrap_DomainEmailSeq(const Registry::Notification::DomainEmailSeq &src, std::map<unsigned long long, std::set<std::string> > &dst);

/**
 * \brief convert implementation exception to interface exception
 *
 * \param src  implementation exception
 */
void raise_DOMAIN_EMAIL_VALIDATION_ERROR(const Admin::Notification::DomainEmailValidationError &src);

} // namespace CorbaConversion

#endif
