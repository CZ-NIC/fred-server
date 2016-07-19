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

#include "src/corba/util/corba_conversions_string.cc"
#include "src/corba/util/corba_conversions_nullable_types.h"

#include "src/fredlib/notify.h"

#include "src/admin/notification/notification.h"

#include "src/corba/notification/server_i.h"

namespace Registry {

    namespace Notification {

        void Server_i::notify_outzone_unguarded_domain_email_list(const DomainEmailSeq &domain_email_seq) {

            try {

                std::map<unsigned long long, std::set<std::string> > domain_emails_map;

                // convert Corba DomainEmailSeq to C++ map of sets of emails (domain_emails_map)

                for (unsigned long long i = 0; i < domain_email_seq.length(); ++i) {
                    domain_emails_map[static_cast<unsigned long long>(domain_email_seq[i].domain_id)].insert(Corba::unwrap_string(domain_email_seq[i].email));
                }

                Admin::Notification::notify_outzone_unguarded_domain_email_list(domain_emails_map);

            } catch (const Admin::Notification::DomainEmailValidationError &e) {

                DomainEmailSeq_var invalid_domain_email_seq = new DomainEmailSeq();

                unsigned long long size = 0;
                for(std::map<unsigned long long, std::set<std::string> >::const_iterator i = e.invalid_domain_emails_map.begin(); i != e.invalid_domain_emails_map.end(); ++i) {
                    size += i->second.size();
                }

                invalid_domain_email_seq->length(size);

                unsigned long index = 0;
                for(std::map<unsigned long long, std::set<std::string> >::const_iterator i = e.invalid_domain_emails_map.begin(); i != e.invalid_domain_emails_map.end(); ++i) {
                    for(std::set<std::string>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
                        invalid_domain_email_seq[index].domain_id = CORBA::ULongLong(i->first);
                        invalid_domain_email_seq[index].email = Corba::wrap_string(*j);
                        ++index;
                    }
                }

                throw DOMAIN_EMAIL_VALIDATION_ERROR(invalid_domain_email_seq);

            } catch (const Admin::Notification::InternalError &e) {
                throw INTERNAL_SERVER_ERROR();
            } catch (...) {
                throw INTERNAL_SERVER_ERROR();
            }

        }

    }

}

/* vim: set et sw=4 : */
