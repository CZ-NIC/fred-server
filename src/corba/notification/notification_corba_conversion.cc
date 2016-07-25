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
 *  implementation for Notification Corba conversion
 */

#include "src/corba/util/corba_conversions_string.cc"
#include "src/corba/util/corba_conversions_nullable_types.h"

#include "src/corba/Notification.hh"

#include "src/corba/notification/notification_corba_conversion.h"

namespace CorbaConversion {

    void unwrap_notification_emails(const Registry::Notification::DomainEmailSeq &domain_email_seq, std::map<unsigned long long, std::set<std::string> > &domain_emails_map) {
        for (unsigned long long index = 0; index < domain_email_seq.length(); ++index) {
            unsigned long long domain_id = static_cast<unsigned long long>(domain_email_seq[index].domain_id);
            std::string email = Corba::unwrap_string(domain_email_seq[index].email);
            std::set<std::string> &domain_emails = domain_emails_map[domain_id]; // required side-effect: creates the element if it does not exist yet
            if(!email.empty()) {
                domain_emails.insert(email);
            }
        }
    }

    void wrap_notification_emails(const std::map<unsigned long long, std::set<std::string> > &domain_emails_map, Registry::Notification::DomainEmailSeq_var &domain_email_seq) {
        unsigned long long size = 0;
        for(std::map<unsigned long long, std::set<std::string> >::const_iterator src_item = domain_emails_map.begin(); src_item != domain_emails_map.end(); ++src_item) {
            size += src_item->second.size();
        }
        domain_email_seq->length(size);

        unsigned long dst_index = 0;
        for(std::map<unsigned long long, std::set<std::string> >::const_iterator src_item = domain_emails_map.begin(); src_item != domain_emails_map.end(); ++src_item) {
            for(std::set<std::string>::const_iterator email_ptr = src_item->second.begin(); email_ptr != src_item->second.end(); ++email_ptr) {
                domain_email_seq[dst_index].domain_id = CORBA::ULongLong(src_item->first);
                domain_email_seq[dst_index].email = Corba::wrap_string(*email_ptr);
                ++dst_index;
            }
        }
    }

}
