#include "admin/contact/verification/run_all_enqueued_checks.h"
#include "admin/contact/verification/run_first_enqueued_check.h"

namespace  Admin {

    std::vector<std::string> run_all_enqueued_checks(const std::map<std::string, boost::shared_ptr<Admin::ContactVerificationTest> >& _tests) {
        std::vector<std::string> handles;

        while(true) {
            try {
                handles.push_back(
                    run_first_enqueued_check(_tests) );
            } catch (ExceptionNoEnqueuedChecksAvailable& ) {
                break;
            }
        }

        return handles;
    }
}
