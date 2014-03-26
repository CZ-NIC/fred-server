#include "src/admin/contact/verification/update_tests.h"
#include <fredlib/admin_contact_verification.h>

#include "util/log/context.h"

namespace  Admin {
    void update_tests(
        Fred::OperationContext&                  _ctx,
        const uuid&                             _check_handle,
        const vector<pair<string, string> >&    _test_statuses,
        unsigned long long                      _logd_request_id
    ) {
        Logging::Context log("update_tests");

        std::vector<std::string> allowed_statuses = Fred::ContactCheckStatus::get_resolution_awaiting();
        // if current check status is not valid for tests change...
        if(
            std::find(
                allowed_statuses.begin(),
                allowed_statuses.end(),
                Fred::InfoContactCheck(_check_handle).exec(_ctx)
                    .check_state_history.rbegin()->status_handle
            ) == allowed_statuses.end()
        ) {
            throw Admin::ExceptionCheckNotUpdateable();
        }

        try {
            for(vector<pair<string, string> >::const_iterator it = _test_statuses.begin();
                it != _test_statuses.begin();
                ++it
            ) {
                Fred::UpdateContactTest(
                    _check_handle,
                    it->first,
                    it->second,
                    _logd_request_id,
                    Optional<string>()
                ).exec(_ctx);
            }
        } catch(const Fred::ExceptionUnknownCheckHandle&) {
            throw Admin::ExceptionUnknownCheckHandle();
        } catch(const Fred::ExceptionUnknownTestHandle&) {
            throw Admin::ExceptionUnknownTestHandle();
        } catch(const Fred::ExceptionUnknownCheckTestPair&) {
            throw Admin::ExceptionUnknownCheckTestPair();
        } catch(const Fred::ExceptionUnknownTestStatusHandle&) {
            throw Admin::ExceptionUnknownTestStatusHandle();
        }
    }
}
