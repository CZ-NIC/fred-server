#include "admin/contact/verification/create_test_impl_prototypes.h"
#include "admin/contact/verification/test_impl/test_name_syntax.h"
#include "admin/contact/verification/test_impl/test_email_syntax.h"
#include "admin/contact/verification/test_impl/test_phone_syntax.h"
#include "admin/contact/verification/test_impl/test_cz_address_exists.h"
#include "admin/contact/verification/test_impl/test_contactability.h"

#include <boost/make_shared.hpp>

namespace  Admin {

    std::map< std::string, boost::shared_ptr<Admin::ContactVerificationTest> > create_test_impl_prototypes(
        boost::shared_ptr<Fred::Mailer::Manager>   _mailer_manager,
        boost::shared_ptr<Fred::Document::Manager> _document_manager,
        boost::shared_ptr<Fred::Messages::Manager> _message_manager,
        const std::string&                         _cz_address_dataset_path
    ) {
        std::map< std::string, boost::shared_ptr<Admin::ContactVerificationTest> > result;

        {
            boost::shared_ptr<Admin::ContactVerificationTest> temp_ptr(new Admin::ContactVerificationTestNameSyntax);
            result[temp_ptr->get_name()] = temp_ptr;
        }

        {
            boost::shared_ptr<Admin::ContactVerificationTest> temp_ptr(new Admin::ContactVerificationTestEmailSyntax);
            result[temp_ptr->get_name()] = temp_ptr;
        }

        {
            boost::shared_ptr<Admin::ContactVerificationTest> temp_ptr(new Admin::ContactVerificationTestPhoneSyntax);
            result[temp_ptr->get_name()] = temp_ptr;
        }

        {
            boost::shared_ptr<Admin::ContactVerificationTest> temp_ptr(new
                Admin::ContactVerificationTestCzAddress(_cz_address_dataset_path));
            result[temp_ptr->get_name()] = temp_ptr;
        }

        {
            boost::shared_ptr<Admin::ContactVerificationTest> temp_ptr
                = boost::make_shared<ContactVerificationTestContactability>(
                    _mailer_manager, _document_manager, _message_manager );

            result[temp_ptr->get_name()] = temp_ptr;
        }

        return result;
    }
}
