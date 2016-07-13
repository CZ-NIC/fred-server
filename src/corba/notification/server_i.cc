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

                std::vector<std::pair<unsigned long long, std::string> > domain_email_list;

                // convert Corba DomainEmailSeq to C++ vector of pairs (domain_email_list)
                for (unsigned long long i = 0; i < domain_email_seq.length(); ++i) {
                    domain_email_list.push_back(
                        std::make_pair(
                            static_cast<unsigned long long>(domain_email_seq[i].domain_id),
                            Corba::unwrap_string(domain_email_seq[i].email)
                        )
                    );
                }

                Admin::Notification::notify_outzone_unguarded_domain_email_list(domain_email_list);

            } catch (const Admin::Notification::DomainEmailValidationError &e) {

                DomainEmailSeq_var invalid_domain_email_seq = new DomainEmailSeq();

                unsigned long index = 0;
                invalid_domain_email_seq->length(e.invalid_domain_email_list.size());
                for (std::vector<std::pair<unsigned long long, std::string> >::const_iterator it = e.invalid_domain_email_list.begin();
                    it != e.invalid_domain_email_list.end();
                    ++it, ++index
                ) {
                    invalid_domain_email_seq[index].domain_id = CORBA::ULongLong(it->first);
                    invalid_domain_email_seq[index].email = Corba::wrap_string(it->second);
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
