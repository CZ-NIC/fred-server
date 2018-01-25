/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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

#include "src/backend/admin/contact/verification/test_impl/test_email_exists.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrable_object/contact/verification/enum_test_status.hh"

#include <boost/algorithm/string.hpp>
#include <boost/scoped_array.hpp>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

namespace Admin {
namespace ContactVerification {

    FACTORY_MODULE_INIT_DEFI(TestEmailExists_init)

    Test::TestRunResult TestEmailExists::run(unsigned long long _history_id) const {
        using std::string;

        TestDataProvider<TestEmailExists> data;
        data.init_data(_history_id);

        boost::trim(data.email_);

        std::vector<std::string> emails;
        boost::algorithm::split(
            emails,
            data.email_,
            boost::is_any_of(",")
        );

        std::vector<std::string> invalid_emails;

        for(std::vector<std::string>::const_iterator it = emails.begin();
            it != emails.end();
            ++it
        ) {
            string email = boost::trim_copy(*it);

            if(email.empty()) {
                invalid_emails.push_back("\"" + *it + "\"");
                continue;
            }

            string host;
            try {
                host = email.substr(email.find('@') + 1);   // +1 <=> cut the '@' as well
            } catch(...) {
                invalid_emails.push_back(*it);
                continue;
            }

            unsigned char* buffer_ptr;
            static const unsigned int buffer_length = 12;   // DNS header is rumored to have 12 bytes

            try {
                buffer_ptr = new unsigned char[buffer_length];
            } catch(const std::exception&) {
                return TestRunResult( LibFred::ContactTestStatus::ERROR, string("runtime error") );
            }
            boost::scoped_array<unsigned char> buffer_scoped(buffer_ptr);
            buffer_ptr = NULL;

            // MX test
            if( res_query(
                    host.c_str(),
                    C_IN,
                    T_MX,
                    buffer_scoped.get(),
                    buffer_length
                ) == -1
            ) {
                // fallback to A record test
                memset(buffer_scoped.get(), 0, buffer_length);

                if( res_query(
                        host.c_str(),
                        C_IN,
                        T_A,
                        buffer_scoped.get(),
                        buffer_length
                    ) == -1
                ) {
                    invalid_emails.push_back(*it);
                    continue;
                }
            }
        }

        if( !invalid_emails.empty() ) {
            return TestRunResult (LibFred::ContactTestStatus::FAIL, "hostname in emails: " + boost::join(invalid_emails, ",") + " couldn't be resolved");
        }

        return TestRunResult( LibFred::ContactTestStatus::OK);
    }
}
}