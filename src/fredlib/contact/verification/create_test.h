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

/**
 *  \file
 *  create contact test
 */

#ifndef CONTACT_VERIFICATION_CREATE_TEST_51547658410_
#define CONTACT_VERIFICATION_CREATE_TEST_51547658410_

#include "fredlib/opexception.h"
#include "fredlib/opcontext.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"

namespace Fred
{

    class CreateContactTest
    {
        std::string         check_handle_;    // to which check (must be already present in db) does this test belong to
        std::string         test_name_;
        Nullable<long long> logd_request_id_; // entry in log_entry database table

    public:
        // constructors
        CreateContactTest(
            const std::string& _check_handle,
            const std::string& _test_name
        );
        CreateContactTest(
            const std::string&  _check_handle,
            const std::string&  _test_name,
            Optional<long long> _logd_request_id
        );

        // setters for optional parameters
        CreateContactTest& set_logd_request_id(long long _logd_request_id);
        CreateContactTest& unset_logd_request_id();

        // exec and serialization
        void exec(OperationContext& ctx);
        friend std::ostream& operator<<(std::ostream& os, const CreateContactTest& i);
        std::string to_string() const;
    };
}
#endif // #include guard end
