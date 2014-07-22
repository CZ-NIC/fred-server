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

#include <boost/regex.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/foreach.hpp>

#include "src/admin/contact/verification/test_impl/test_email_syntax.h"
#include "src/fredlib/contact/verification/enum_test_status.h"

#include <fredlib/contact.h>

#include <boost/algorithm/string/trim.hpp>

namespace Admin {
namespace ContactVerification {

    FACTORY_MODULE_INIT_DEFI(TestEmailSyntax_init)

    Test::TestRunResult TestEmailSyntax::run(unsigned long long _history_id) const {
        TestDataProvider<TestEmailSyntax> data;
        data.init_data(_history_id);

        std::string email = boost::trim_copy(data.email_);

        if ( boost::regex_match(
            email,
            EMAIL_PATTERN )
        ) {
            return TestRunResult(Fred::ContactTestStatus::OK );
        }

        return TestRunResult(Fred::ContactTestStatus::FAIL, std::string("invalid e-mail format") );
    }
}
}
