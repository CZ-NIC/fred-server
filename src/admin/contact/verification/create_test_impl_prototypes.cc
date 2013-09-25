#include "admin/contact/verification/create_test_impl_prototypes.h"
#include "admin/contact/verification/test_impl/test_email_syntax.h"

namespace  Admin {

    std::map< std::string, boost::shared_ptr<Admin::ContactVerificationTest> > create_test_impl_prototypes(void) {
        std::map< std::string, boost::shared_ptr<Admin::ContactVerificationTest> > result;

        boost::shared_ptr<Admin::ContactVerificationTest> temp_ptr(new Admin::ContactVerificationTestEmailSyntax);
        result[temp_ptr->get_name()] = temp_ptr;

        return result;
    }
}
