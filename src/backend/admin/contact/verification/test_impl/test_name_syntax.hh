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

#ifndef TEST_NAME_SYNTAX_HH_D585913B490D498FBDB3F06070B637B7
#define TEST_NAME_SYNTAX_HH_D585913B490D498FBDB3F06070B637B7

#include "src/backend/admin/contact/verification/test_impl/test_interface.hh"

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

class TestNameSyntax : public Test
{
public:
    TestRunResult run(unsigned long long _history_id) const override;
};

template <>
std::string test_name<TestNameSyntax>();

template <>
struct TestDataProvider<TestNameSyntax> : TestDataProvider_common
{
    void store_data(const LibFred::InfoContactOutput& _data) override;
    std::vector<std::string> get_string_data() const override;

    std::string name_;
};

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif//TEST_NAME_SYNTAX_HH_D585913B490D498FBDB3F06070B637B7
