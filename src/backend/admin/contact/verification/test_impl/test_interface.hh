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
 *  @file
 *  contact verification tests interface
 */

#ifndef TEST_INTERFACE_HH_A08924E6431F4104B6BCBAE4FCD18BA7
#define TEST_INTERFACE_HH_A08924E6431F4104B6BCBAE4FCD18BA7

#include "src/libfred/registrable_object/contact/check_contact.hh"
#include "src/libfred/registrable_object/contact/copy_contact.hh"
#include "src/libfred/registrable_object/contact/create_contact.hh"
#include "src/libfred/registrable_object/contact/delete_contact.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrable_object/contact/info_contact_diff.hh"
#include "src/libfred/registrable_object/contact/merge_contact.hh"
#include "src/libfred/registrable_object/contact/update_contact.hh"
#include "src/util/factory.hh"
#include "src/util/optional_value.hh"

#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>

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
        Optional<std::string> error_message;
        // XXX hopefuly one day related mail and messages will be unified
        std::set<unsigned long long> related_mail_archive_ids;
        std::set<unsigned long long> related_message_archive_ids;


        TestRunResult(
                const std::string& _status,
                const Optional<std::string>& _error_msg = Optional<std::string>(),
                const std::set<unsigned long long>& _related_mail_archive_ids = std::set<unsigned long long>(),
                const std::set<unsigned long long>& _related_message_archive_ids =
                        std::set<unsigned long long>());
    };

    /**
     * @return final status of the test, optional error message and optional related states and messages
     */
    virtual TestRunResult run(unsigned long long _history_id) const = 0;


    virtual ~Test();


    static std::string registration_name();


};

struct TestDataProvider_intf
{
    virtual TestDataProvider_intf& init_data(unsigned long long _contact_history_id) = 0;

    virtual std::vector<std::string> get_string_data() const = 0;


    virtual ~TestDataProvider_intf()
    {
    }

};

template <typename T_test>
struct _inheritTestRegName
{
    static std::string registration_name()
    {
        return T_test::registration_name();
    }

};

struct TestDataProvider_common
        : public TestDataProvider_intf
{
private:
    virtual LibFred::InfoContactOutput get_data(unsigned long long _contact_history_id);

    virtual void store_data(const LibFred::InfoContactOutput& _data) = 0;


public:
    virtual TestDataProvider_intf& init_data(unsigned long long _contact_history_id);


};

template <typename Test>
struct TestDataProvider { };

typedef Util::Factory<Test, Util::ClassCreator<Test>> test_factory;
typedef Util::Factory<TestDataProvider_intf, Util::ClassCreator<TestDataProvider_intf>>
        test_data_provider_factory;

template <typename Test_impl>
class test_auto_registration
        : Util::FactoryAutoRegister<Test, Test_impl>,
          Util::FactoryAutoRegister<TestDataProvider_intf, TestDataProvider<Test_impl>>
{
};

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif
