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
#include "src/backend/admin/contact/verification/test_impl/test_phone_syntax.hh"

#include "libfred/registrable_object/contact/verification/enum_test_status.hh"

#include <boost/algorithm/string/trim.hpp>
#include <boost/regex.hpp>


namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

Test::TestRunResult TestPhoneSyntax::run(unsigned long long _history_id) const
{
    TestDataProvider<TestPhoneSyntax> data;
    data.init_data(_history_id);

    std::string trimmed_telephone =  boost::algorithm::trim_copy(static_cast<std::string>(data.phone_));

    if (trimmed_telephone.empty())
    {
        return TestRunResult{
                LibFred::ContactTestStatus::SKIPPED,
                "optional telephone is empty"};
    }

    const auto phone_pattern = boost::regex{"^\\+[0-9]{1,3}\\.[0-9]{1,14}$"};
    if (boost::regex_match(
                // if Nullable is NULL then this casts returns empty string
                trimmed_telephone,
                phone_pattern))
    {
        return TestRunResult{LibFred::ContactTestStatus::OK};
    }

    return TestRunResult{
            LibFred::ContactTestStatus::FAIL,
            "invalid phone format"};
}

void TestDataProvider<TestPhoneSyntax>::store_data(const LibFred::InfoContactOutput& _data)
{
    if (!_data.info_contact_data.telephone.isnull())
    {
        phone_ = _data.info_contact_data.telephone.get_value_or_default();
    }
}

std::vector<std::string> TestDataProvider<TestPhoneSyntax>::get_string_data() const
{
    return {phone_};
}

template <>
std::string test_name<TestPhoneSyntax>()
{
    return "phone_syntax";
}

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
