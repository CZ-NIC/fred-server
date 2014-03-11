#include "src/admin/contact/verification/run_all_enqueued_checks.h"
#include "src/admin/contact/verification/run_first_enqueued_check.h"

#include "util/log/context.h"

namespace  Admin {

    std::vector<std::string> run_all_enqueued_checks(
        const std::map<std::string, boost::shared_ptr<Admin::ContactVerification::Test> >& _tests,
        Optional<unsigned long long> _logd_request_id
    ) {
        Logging::Context log("run_all_enqueued_checks");

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
