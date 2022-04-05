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

#ifndef TEST_INTERFACE_HH_A08924E6431F4104B6BCBAE4FCD18BA7
#define TEST_INTERFACE_HH_A08924E6431F4104B6BCBAE4FCD18BA7

#include "util/factory.hh"
#include "util/optional_value.hh"

#include "libfred/registrable_object/contact/info_contact_output.hh"

#include <set>
#include <string>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

class Test
{
public:
    struct TestRunResult
    {
        std::string status;
        Optional<std::string> error_message = Optional<std::string>{};
        // XXX hopefully one day related mail and messages will be unified
        std::set<unsigned long long> related_mail_archive_ids = {};
        std::set<unsigned long long> related_message_archive_ids = {};
    };

    virtual ~Test();

    /**
     * @return final status of the test, optional error message and optional related states and messages
     */
    virtual TestRunResult run(unsigned long long _history_id) const = 0;
};

template <typename T>
std::string test_name();

struct TestDataProvider_intf
{
    virtual ~TestDataProvider_intf() { }
    virtual TestDataProvider_intf& init_data(unsigned long long _contact_history_id) = 0;
    virtual std::vector<std::string> get_string_data() const = 0;
};

class TestDataProvider_common : public TestDataProvider_intf
{
public:
    virtual TestDataProvider_intf& init_data(unsigned long long _contact_history_id);
private:
    virtual LibFred::InfoContactOutput get_data(unsigned long long _contact_history_id);
    virtual void store_data(const LibFred::InfoContactOutput& _data) = 0;
};

template <typename> struct TestDataProvider;

using test_factory = Util::Factory<Test> ;
using test_data_provider_factory = Util::Factory<TestDataProvider_intf>;

const test_factory& get_default_test_factory();
const test_data_provider_factory& get_default_test_data_provider_factory();

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif//TEST_INTERFACE_HH_A08924E6431F4104B6BCBAE4FCD18BA7
