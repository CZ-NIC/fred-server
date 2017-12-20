#include "src/backend/admin/contact/verification/create_test_impl_prototypes.hh"
#include "src/backend/admin/contact/verification/test_impl/test_name_syntax.hh"
#include "src/backend/admin/contact/verification/test_impl/test_email_syntax.hh"
#include "src/backend/admin/contact/verification/test_impl/test_phone_syntax.hh"
#include "src/backend/admin/contact/verification/test_impl/test_cz_address_exists.hh"
#include "src/backend/admin/contact/verification/test_impl/test_contactability.hh"
#include "src/backend/admin/contact/verification/test_impl/test_email_exists.hh"
#include "src/backend/admin/contact/verification/test_impl/test_send_letter.hh"
#include "src/backend/admin/contact/verification/test_impl/test_email_exists_for_managed_zones.hh"
#include "src/backend/admin/contact/verification/exceptions.hh"

#include "src/util/log/context.hh"


namespace  Admin {

    std::map< std::string, std::shared_ptr<ContactVerification::Test> > create_test_impl_prototypes(
        std::shared_ptr<LibFred::Mailer::Manager>   _mailer_manager,
        std::shared_ptr<LibFred::Document::Manager> _document_manager,
        std::shared_ptr<LibFred::Messages::Manager> _message_manager,
        const std::string&                         _cz_address_dataset_path
    ) {
        Logging::Context log("create_test_impl_prototypes");

        std::map< std::string, std::shared_ptr<ContactVerification::Test> > result;

        try {
            result[ContactVerification::TestNameSyntax::registration_name()] =
                ContactVerification::test_factory::instance_ref()
                    .create_sh_ptr(ContactVerification::TestNameSyntax::registration_name());

            result[ContactVerification::TestEmailSyntax::registration_name()] =
                ContactVerification::test_factory::instance_ref()
                    .create_sh_ptr(ContactVerification::TestEmailSyntax::registration_name());

            result[ContactVerification::TestPhoneSyntax::registration_name()] =
                ContactVerification::test_factory::instance_ref()
                    .create_sh_ptr(ContactVerification::TestPhoneSyntax::registration_name());

            result[ContactVerification::TestEmailExistsForManagedZones::registration_name()] =
                ContactVerification::test_factory::instance_ref()
                    .create_sh_ptr(ContactVerification::TestEmailExistsForManagedZones::registration_name());

            {
                result[ContactVerification::TestCzAddress::registration_name()] =
                    ContactVerification::test_factory::instance_ref()
                        .create_sh_ptr(ContactVerification::TestCzAddress::registration_name());

                ContactVerification::TestCzAddress * instance =
                    dynamic_cast<ContactVerification::TestCzAddress *>(
                        result[ContactVerification::TestCzAddress::registration_name()].get() );

                if(instance == NULL) {
                    throw Admin::ExceptionTestImplementationError();
                }
                instance->set_mvcr_address_xml_filename(_cz_address_dataset_path);
            }

            {
                result[ContactVerification::TestContactability::registration_name()] =
                    ContactVerification::test_factory::instance_ref()
                        .create_sh_ptr(ContactVerification::TestContactability::registration_name());

                ContactVerification::TestContactability * instance =
                    dynamic_cast<ContactVerification::TestContactability *>(
                        result[ContactVerification::TestContactability::registration_name()].get() );

                if(instance == NULL) {
                    throw Admin::ExceptionTestImplementationError();
                }
                instance->set_document_file_manager(_document_manager)
                    .set_email_manager(_mailer_manager)
                    .set_letter_manager(_message_manager );
            }

            result[ContactVerification::TestEmailExists::registration_name()] =
                ContactVerification::test_factory::instance_ref()
                    .create_sh_ptr(ContactVerification::TestEmailExists::registration_name());

            {
                result[ContactVerification::TestSendLetter::registration_name()] =
                    ContactVerification::test_factory::instance_ref()
                        .create_sh_ptr(ContactVerification::TestSendLetter::registration_name());

                ContactVerification::TestSendLetter * instance =
                    dynamic_cast<ContactVerification::TestSendLetter *>(
                        result[ContactVerification::TestSendLetter::registration_name()].get() );

                if(instance == NULL) {
                    throw Admin::ExceptionTestImplementationError();
                }
                instance->set_document_file_manager(_document_manager)
                    .set_letter_manager(_message_manager );
            }
        } catch (const std::bad_cast& ) {
            throw Admin::ExceptionTestImplementationError();
        }

        return result;
    }
}
