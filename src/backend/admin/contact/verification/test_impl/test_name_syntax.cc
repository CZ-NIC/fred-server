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

#include "src/backend/admin/contact/verification/test_impl/test_name_syntax.hh"

#include "libfred/registrable_object/contact/verification/enum_test_status.hh"

#include <boost/algorithm/string/trim.hpp>


namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

Test::TestRunResult TestNameSyntax::run(unsigned long long _history_id) const
{
    TestDataProvider<TestNameSyntax> data;
    data.init_data(_history_id);

    std::string name =  boost::algorithm::trim_copy(static_cast<std::string>(data.name_));

    if (name.find(' ') == std::string::npos)
    {
        return TestRunResult{
                LibFred::ContactTestStatus::FAIL,
                "name has to contain at least two words separated by space"};
    }
    return TestRunResult{LibFred::ContactTestStatus::OK};
}

void TestDataProvider<TestNameSyntax>::store_data(const LibFred::InfoContactOutput& _data)
{
    if (!_data.info_contact_data.name.isnull())
    {
        name_ = _data.info_contact_data.name.get_value_or_default();
    }
}

std::vector<std::string> TestDataProvider<TestNameSyntax>::get_string_data() const
{
    return {name_};
}

template <>
std::string test_name<TestNameSyntax>()
{
    return "name_syntax";
}

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
