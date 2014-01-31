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

#include "src/admin/contact/verification/test_impl/test_email_exists.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/verification/enum_test_status.h"

#include <boost/algorithm/string.hpp>
#include <boost/scoped_array.hpp>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

namespace Admin {
    ContactVerificationTest::T_run_result ContactVerificationTestEmailExists::run(long _history_id) const {
        using std::string;
        using std::vector;

        Fred::OperationContext ctx;
        const Fred::InfoContactData& contact_data = Fred::HistoryInfoContactByHistoryid(_history_id).exec(ctx).info_contact_data;

        if(contact_data.email.isnull()) {
            return make_result( Fred::ContactTestStatus::FAIL, string("missing email") );
        }

        std::string email = boost::trim_copy(static_cast<std::string>(contact_data.email));

        if(email.length() == 0) {
            return make_result( Fred::ContactTestStatus::FAIL, string("empty email") );
        }

        std::string host;
        try {
            host = email.substr(email.find('@') + 1);   // +1 <=> cut the '@' as well
        } catch(...) {
            return make_result( Fred::ContactTestStatus::FAIL, string("invalid email format") );
        }

        unsigned char* buffer_ptr;
        try {
            buffer_ptr = new unsigned char[100];
        } catch(std::exception& e) {
            return make_result( Fred::ContactTestStatus::ERROR, string("runtime error") );
        }
        boost::scoped_array<unsigned char> buffer_scoped(buffer_ptr);
        buffer_ptr = NULL;

        // MX test
        int result_len = res_query(
            host.c_str(),
            C_IN,
            T_MX,
            buffer_scoped.get(),
            100
        );

        if(result_len != -1) {
            return make_result( Fred::ContactTestStatus::OK);
        }

        // fallback to A record test
        memset(buffer_scoped.get(), 0, 100);
        result_len = res_query(
            host.c_str(),
            C_IN,
            T_A,
            buffer_scoped.get(),
            100
        );

        if(result_len != -1) {
            return make_result( Fred::ContactTestStatus::OK);
        }

        return make_result( Fred::ContactTestStatus::FAIL, string("hostname in email couldn't be resolved") );
    }
}
