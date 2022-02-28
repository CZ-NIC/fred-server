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

#include "src/backend/admin/contact/verification/create_test_impl_prototypes.hh"

#include "src/backend/admin/contact/verification/exceptions.hh"
#include "src/backend/admin/contact/verification/test_impl/test_contactability.hh"
#include "src/backend/admin/contact/verification/test_impl/test_cz_address_exists.hh"
#include "src/backend/admin/contact/verification/test_impl/test_email_exists.hh"
#include "src/backend/admin/contact/verification/test_impl/test_email_exists_for_managed_zones.hh"
#include "src/backend/admin/contact/verification/test_impl/test_email_syntax.hh"
#include "src/backend/admin/contact/verification/test_impl/test_name_syntax.hh"
#include "src/backend/admin/contact/verification/test_impl/test_phone_syntax.hh"
#include "src/backend/admin/contact/verification/test_impl/test_send_letter.hh"

#include "util/log/context.hh"


namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

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

template <typename Producer>
Producer& get_specific_producer(const Util::Factory<Test>& factory)
{
    Producer* producer = dynamic_cast<Producer*>(&(factory[test_name<Producer>()]));
    if (producer == nullptr)
    {
        throw ExceptionTestImplementationError{};
    }
    return *producer;
}

} // namespace Fred::Backend::Admin::Contact::Verification::{anonymous}

Util::Factory<Test> create_test_impl_prototypes(
        std::shared_ptr<LibFred::Mailer::Manager> _mailer_manager,
        std::shared_ptr<LibFred::Document::Manager> _document_manager,
        std::shared_ptr<LibFred::Messages::Manager> _message_manager,
        const std::string& _cz_address_dataset_path)
{
    Logging::Context log("create_test_impl_prototypes");
    try
    {
        Util::Factory<Test> factory{};
        factory.add_producer(make_test_producer<TestContactability>())
               .add_producer(make_test_producer<TestCzAddress>())
               .add_producer(make_test_producer<TestEmailExistsForManagedZones>())
               .add_producer(make_test_producer<TestEmailExists>())
               .add_producer(make_test_producer<TestEmailSyntax>())
               .add_producer(make_test_producer<TestNameSyntax>())
               .add_producer(make_test_producer<TestPhoneSyntax>())
               .add_producer(make_test_producer<TestSendLetter>());
        get_specific_producer<TestCzAddress>(factory)
                .set_mvcr_address_xml_filename(_cz_address_dataset_path);
        get_specific_producer<TestContactability>(factory)
                .set_document_file_manager(_document_manager)
                .set_email_manager(_mailer_manager)
                .set_letter_manager(_message_manager);
        get_specific_producer<TestSendLetter>(factory)
                .set_document_file_manager(_document_manager)
                .set_letter_manager(_message_manager);
        return factory;
    }
    catch (const std::bad_cast&)
    {
        throw ExceptionTestImplementationError{};
    }
}

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
