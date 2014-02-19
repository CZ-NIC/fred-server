#include "src/admin/contact/verification/create_test_impl_prototypes.h"
#include "src/admin/contact/verification/test_impl/test_name_syntax.h"
#include "src/admin/contact/verification/test_impl/test_email_syntax.h"
#include "src/admin/contact/verification/test_impl/test_phone_syntax.h"
#include "src/admin/contact/verification/test_impl/test_cz_address_exists.h"
#include "src/admin/contact/verification/test_impl/test_contactability.h"
#include "src/admin/contact/verification/test_impl/test_email_exists.h"

#include <boost/make_shared.hpp>

namespace  Admin {

    std::map< std::string, boost::shared_ptr<ContactVerification::Test> > create_test_impl_prototypes(
        boost::shared_ptr<Fred::Mailer::Manager>   _mailer_manager,
        boost::shared_ptr<Fred::Document::Manager> _document_manager,
        boost::shared_ptr<Fred::Messages::Manager> _message_manager,
        const std::string&                         _cz_address_dataset_path
    ) {
        std::map< std::string, boost::shared_ptr<ContactVerification::Test> > result;

        result[ContactVerification::TestNameSyntax::registration_name()] =
            ContactVerification::test_factory::instance_ref()
                .create_sh_ptr(ContactVerification::TestNameSyntax::registration_name());

        result[ContactVerification::TestEmailSyntax::registration_name()] =
            ContactVerification::test_factory::instance_ref()
                .create_sh_ptr(ContactVerification::TestEmailSyntax::registration_name());

        result[ContactVerification::TestPhoneSyntax::registration_name()] =
            ContactVerification::test_factory::instance_ref()
                .create_sh_ptr(ContactVerification::TestPhoneSyntax::registration_name());

        result[ContactVerification::TestCzAddress::registration_name()] =
                ContactVerification::test_factory::instance_ref()
                    .create_sh_ptr(ContactVerification::TestCzAddress::registration_name());

        dynamic_cast<ContactVerification::TestCzAddress *>(
            result[ContactVerification::TestCzAddress::registration_name()].get()
        )->set_mvcr_address_xml_filename(_cz_address_dataset_path);

        result[ContactVerification::TestContactability::registration_name()] =
            ContactVerification::test_factory::instance_ref()
                .create_sh_ptr(ContactVerification::TestContactability::registration_name());

        dynamic_cast<ContactVerification::TestContactability *>(
            result[ContactVerification::TestContactability::registration_name()].get()
        )->set_document_file_manager(_document_manager)
        .set_email_manager(_mailer_manager)
        .set_letter_manager(_message_manager );

        result[ContactVerification::TestEmailExists::registration_name()] =
            ContactVerification::test_factory::instance_ref()
                .create_sh_ptr(ContactVerification::TestEmailExists::registration_name());

        return result;
    }
}
