#include "admin/contact/verification/run_all_enqueued_checks.h"
#include "admin/contact/verification/run_first_enqueued_check.h"

namespace  Admin {

    std::vector<std::string> run_all_enqueued_checks(const std::map<std::string, boost::shared_ptr<Admin::ContactVerificationTest> >& _tests, Optional<long long> _logd_request_id) {
        std::vector<std::string> handles;
        Optional<std::string> temp_handle;

        while(true) {
            temp_handle = run_first_enqueued_check(_tests, _logd_request_id );
            if(temp_handle.isset() == false) {
                break;
            }

            handles.push_back(static_cast<std::string>(temp_handle));
        }

        return handles;
    }
}
