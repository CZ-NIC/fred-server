/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
 *  implementation for Notification Corba conversion
 */

#include "src/util/corba_conversion.hh"
#include "src/bin/corba/util/corba_conversions_string.hh"
#include "src/bin/corba/util/corba_conversions_nullable_types.hh"

#include "src/bin/corba/Notification.hh"

#include "src/bin/corba/notification/notification_corba_conversion.hh"

namespace CorbaConversion {

void unwrap_DomainEmailSeq(const Registry::Notification::DomainEmailSeq &domain_email_seq, std::map<unsigned long long, std::set<std::string> > &domain_emails_map) {
    for (unsigned long long index = 0; index < domain_email_seq.length(); ++index) {
        unsigned long long domain_id;
        CorbaConversion::unwrap_int(domain_email_seq[index].domain_id, domain_id);
        const std::string email = LibFred::Corba::unwrap_string(domain_email_seq[index].email);
        domain_emails_map[domain_id].insert(email);
    }
}

/**
 * \brief  convert implementation type to interface type
 *
 * \param domain_emails_map  emails by domain_id
 * \param domain_email_seq   sequence of {domain_id, email}
 */
void wrap_map_unsigned_long_long_set_string(const std::map<unsigned long long, std::set<std::string> > &domain_emails_map, Registry::Notification::DomainEmailSeq_var &domain_email_seq) {
    unsigned long long domain_email_seq_length = 0;
    for(std::map<unsigned long long, std::set<std::string> >::const_iterator src_item = domain_emails_map.begin(); src_item != domain_emails_map.end(); ++src_item) {
        domain_email_seq_length += src_item->second.size();
    }
    domain_email_seq->length(domain_email_seq_length);

    unsigned long dst_index = 0;
    for(std::map<unsigned long long, std::set<std::string> >::const_iterator src_item = domain_emails_map.begin(); src_item != domain_emails_map.end(); ++src_item) {
        for(std::set<std::string>::const_iterator email_ptr = src_item->second.begin(); email_ptr != src_item->second.end(); ++email_ptr) {
        	CorbaConversion::wrap_int(src_item->first, domain_email_seq[dst_index].domain_id);
            domain_email_seq[dst_index].email = CorbaConversion::wrap_string(*email_ptr);
            ++dst_index;
        }
    }
}

static void wrap_DomainEmailValidationError(const Admin::Notification::DomainEmailValidationError &src, Registry::Notification::DOMAIN_EMAIL_VALIDATION_ERROR &dst) {
    Registry::Notification::DomainEmailSeq_var domain_invalid_email_seq = new Registry::Notification::DomainEmailSeq();
    CorbaConversion::wrap_map_unsigned_long_long_set_string(src.domain_invalid_emails_map, domain_invalid_email_seq);
    dst = Registry::Notification::DOMAIN_EMAIL_VALIDATION_ERROR(domain_invalid_email_seq);
}

void raise_DOMAIN_EMAIL_VALIDATION_ERROR(const Admin::Notification::DomainEmailValidationError &src) {

    Registry::Notification::DOMAIN_EMAIL_VALIDATION_ERROR e;
    try {
        wrap_DomainEmailValidationError(src, e);
    }
    catch(...) {
        throw Registry::Notification::INTERNAL_SERVER_ERROR();
    }
    throw e;
}

} // namespace CorbaConversion
