/*
 * Copyright (C) 2013-2022  CZ.NIC, z. s. p. o.
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

#include "src/backend/admin/contact/verification/test_impl/test_email_syntax.hh"

#include "libfred/registrable_object/contact/verification/enum_test_status.hh"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <boost/regex.hpp>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

Test::TestRunResult TestEmailSyntax::run(unsigned long long _history_id) const
{
    TestDataProvider<TestEmailSyntax> data;
    data.init_data(_history_id);

    std::string email = boost::trim_copy(data.email_);

    /* legacy compatibility
     * old e-mail data in register can be multiple e-mail addresses separated by commas ","
     */
    const auto email_pattern = boost::regex{
                  "^"
                  // first - mandatory e-mail
                  "[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+(\\.[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+)*"
                  "@(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,6}\\.?"
                  "(?:"   // list of ...
                  ","       // < ... comma separated ...
                  // ... other addresses ...
                  "[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+(\\.[-!#$%&'*+/=?^_`{}|~0-9A-Za-z]+)*"
                  "@(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,6}\\.?"
                  ")*"   // < ... which is only optional
                  "$",
                  boost::regex::icase};
    if (boost::regex_match(email, email_pattern))
    {
        return TestRunResult{LibFred::ContactTestStatus::OK};
    }

    return TestRunResult{
            LibFred::ContactTestStatus::FAIL,
            "invalid e-mail format"};
}

void TestDataProvider<TestEmailSyntax>::store_data(const LibFred::InfoContactOutput& _data)
{
    if (!_data.info_contact_data.email.isnull())
    {
        email_ = _data.info_contact_data.email.get_value_or_default();
    }
}

std::vector<std::string> TestDataProvider<TestEmailSyntax>::get_string_data() const
{
    return {email_};
}

template <>
std::string test_name<TestEmailSyntax>()
{
    return "email_syntax";
}

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
