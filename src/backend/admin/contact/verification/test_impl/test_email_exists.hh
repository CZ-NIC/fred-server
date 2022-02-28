/*
 * Copyright (C) 2014-2022  CZ.NIC, z. s. p. o.
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

#ifndef TEST_EMAIL_EXISTS_HH_0A7A8925A770410AB0036AFDC9A11442
#define TEST_EMAIL_EXISTS_HH_0A7A8925A770410AB0036AFDC9A11442

#include "src/backend/admin/contact/verification/test_impl/test_interface.hh"


namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

class TestEmailExists : public Test
{
public:
    TestRunResult run(unsigned long long _history_id) const override;
};

template <>
std::string test_name<TestEmailExists>();

template <>
struct TestDataProvider<TestEmailExists> : TestDataProvider_common
{
    void store_data(const LibFred::InfoContactOutput& _data) override;
    std::vector<std::string> get_string_data() const override;

    std::string email_;
};

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif
