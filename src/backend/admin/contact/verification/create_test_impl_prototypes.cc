/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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

std::map<std::string, std::shared_ptr<Test>> create_test_impl_prototypes(
        std::shared_ptr<LibFred::Mailer::Manager> _mailer_manager,
        std::shared_ptr<LibFred::Document::Manager> _document_manager,
        std::shared_ptr<LibFred::Messages::Manager> _message_manager,
        const std::string& _cz_address_dataset_path)
{
    Logging::Context log("create_test_impl_prototypes");

    std::map<std::string, std::shared_ptr<Test> > result;

    try
    {
        result[TestNameSyntax::registration_name()] =
            test_factory::instance_ref()
            .create_sh_ptr(TestNameSyntax::registration_name());

        result[TestEmailSyntax::registration_name()] =
            test_factory::instance_ref()
            .create_sh_ptr(TestEmailSyntax::registration_name());

        result[TestPhoneSyntax::registration_name()] =
            test_factory::instance_ref()
            .create_sh_ptr(TestPhoneSyntax::registration_name());

        result[TestEmailExistsForManagedZones::registration_name()] =
            test_factory::instance_ref()
            .create_sh_ptr(TestEmailExistsForManagedZones::registration_name());

        {
            result[TestCzAddress::registration_name()] =
                test_factory::instance_ref()
                .create_sh_ptr(TestCzAddress::registration_name());

            TestCzAddress* instance =
                dynamic_cast<TestCzAddress*>(
                    result[TestCzAddress::registration_name()].get());

            if (instance == NULL)
            {
                throw ExceptionTestImplementationError();
            }

            instance->set_mvcr_address_xml_filename(_cz_address_dataset_path);
        }

        {
            result[TestContactability::registration_name()] =
                test_factory::instance_ref()
                .create_sh_ptr(TestContactability::registration_name());

            TestContactability* instance =
                dynamic_cast<TestContactability*>(
                    result[TestContactability::registration_name()].get());

            if (instance == NULL)
            {
                throw ExceptionTestImplementationError();
            }
            instance->set_document_file_manager(_document_manager)
            .set_email_manager(_mailer_manager)
            .set_letter_manager(_message_manager);
        }

        result[TestEmailExists::registration_name()] =
            test_factory::instance_ref()
            .create_sh_ptr(TestEmailExists::registration_name());

        {
            result[TestSendLetter::registration_name()] =
                test_factory::instance_ref()
                .create_sh_ptr(TestSendLetter::registration_name());

            TestSendLetter* instance =
                dynamic_cast<TestSendLetter*>(
                    result[TestSendLetter::registration_name()].get());

            if (instance == NULL)
            {
                throw ExceptionTestImplementationError();
            }
            instance->set_document_file_manager(_document_manager)
            .set_letter_manager(_message_manager);
        }
    }
    catch (const std::bad_cast&)
    {
        throw ExceptionTestImplementationError();
    }

    return result;
}

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
