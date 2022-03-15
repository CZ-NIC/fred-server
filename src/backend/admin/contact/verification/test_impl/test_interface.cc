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

#include "src/backend/admin/contact/verification/test_impl/test_interface.hh"

#include "src/backend/admin/contact/verification/test_impl/test_contactability.hh"
#include "src/backend/admin/contact/verification/test_impl/test_cz_address_exists.hh"
#include "src/backend/admin/contact/verification/test_impl/test_email_exists_for_managed_zones.hh"
#include "src/backend/admin/contact/verification/test_impl/test_email_exists.hh"
#include "src/backend/admin/contact/verification/test_impl/test_email_syntax.hh"
#include "src/backend/admin/contact/verification/test_impl/test_name_syntax.hh"
#include "src/backend/admin/contact/verification/test_impl/test_phone_syntax.hh"
#include "src/backend/admin/contact/verification/test_impl/test_send_letter.hh"

#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/info_contact.hh"

#include <utility>


namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

Test::~Test() { }

LibFred::InfoContactOutput TestDataProvider_common::get_data(unsigned long long _contact_history_id)
{
    LibFred::OperationContextCreator ctx;

    return LibFred::InfoContactHistoryByHistoryid(_contact_history_id).exec(ctx);
}

TestDataProvider_intf& TestDataProvider_common::init_data(unsigned long long _contact_history_id)
{
    this->store_data(this->get_data(_contact_history_id));

    return *this;
}

namespace {

template <typename T, typename ...Args>
std::pair<std::string, std::unique_ptr<Test>> make_test_producer(Args&& ...args)
{
    return {test_name<T>(), std::make_unique<T>(std::forward<Args>(args)...)};
}

template <typename T, typename ...Args>
std::pair<std::string, std::unique_ptr<TestDataProvider_intf>> make_test_data_provider_producer(Args&& ...args)
{
    return {test_name<T>(), std::make_unique<TestDataProvider<T>>(std::forward<Args>(args)...)};
}

} // namespace Fred::Backend::Admin::Contact::Verification::{anonymous}

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

using namespace Fred::Backend::Admin::Contact::Verification;

const test_factory& Fred::Backend::Admin::Contact::Verification::get_default_test_factory()
{
    static thread_local const auto factory = []()
    {
        test_factory factory{};
        factory.add_producer(make_test_producer<TestContactability>())
               .add_producer(make_test_producer<TestCzAddress>())
               .add_producer(make_test_producer<TestEmailExistsForManagedZones>())
               .add_producer(make_test_producer<TestEmailExists>())
               .add_producer(make_test_producer<TestEmailSyntax>())
               .add_producer(make_test_producer<TestNameSyntax>())
               .add_producer(make_test_producer<TestPhoneSyntax>())
               .add_producer(make_test_producer<TestSendLetter>());
        return factory;
    }();
    return factory;
}

const test_data_provider_factory& Fred::Backend::Admin::Contact::Verification::get_default_test_data_provider_factory()
{
    static thread_local const auto factory = []()
    {
        test_data_provider_factory factory{};
        factory.add_producer(make_test_data_provider_producer<TestContactability>())
               .add_producer(make_test_data_provider_producer<TestCzAddress>())
               .add_producer(make_test_data_provider_producer<TestEmailExistsForManagedZones>())
               .add_producer(make_test_data_provider_producer<TestEmailExists>())
               .add_producer(make_test_data_provider_producer<TestEmailSyntax>())
               .add_producer(make_test_data_provider_producer<TestNameSyntax>())
               .add_producer(make_test_data_provider_producer<TestPhoneSyntax>())
               .add_producer(make_test_data_provider_producer<TestSendLetter>());
        return factory;
    }();
    return factory;
}
