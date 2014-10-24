#include "src/admin/contact/verification/create_test_impl_prototypes.h"
#include "src/admin/contact/verification/test_impl/test_name_syntax.h"
#include "src/admin/contact/verification/test_impl/test_email_syntax.h"
#include "src/admin/contact/verification/test_impl/test_phone_syntax.h"
#include "src/admin/contact/verification/test_impl/test_cz_address_exists.h"
#include "src/admin/contact/verification/test_impl/test_contactability.h"
#include "src/admin/contact/verification/test_impl/test_email_exists.h"
#include "src/admin/contact/verification/test_impl/test_send_letter.h"
#include "src/admin/contact/verification/test_impl/test_email_exists_for_managed_zones.h"
#include "src/admin/contact/verification/exceptions.h"

#include "util/log/context.h"

#include <boost/make_shared.hpp>

namespace  Admin {

    std::map< std::string, boost::shared_ptr<ContactVerification::Test> > create_test_impl_prototypes(
        boost::shared_ptr<Fred::Mailer::Manager>   _mailer_manager,
        boost::shared_ptr<Fred::Document::Manager> _document_manager,
        boost::shared_ptr<Fred::Messages::Manager> _message_manager,
        const std::string&                         _cz_address_dataset_path
    ) {
        Logging::Context log("create_test_impl_prototypes");

        std::map< std::string, boost::shared_ptr<ContactVerification::Test> > result;

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
