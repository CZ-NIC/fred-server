#include "src/backend/admin/contact/verification/update_tests.hh"

#include "src/libfred/registrable_object/contact/verification/create_check.hh"
#include "src/libfred/registrable_object/contact/verification/create_test.hh"
#include "src/libfred/registrable_object/contact/verification/enum_check_status.hh"
#include "src/libfred/registrable_object/contact/verification/enum_test_status.hh"
#include "src/libfred/registrable_object/contact/verification/enum_testsuite_handle.hh"
#include "src/libfred/registrable_object/contact/verification/exceptions.hh"
#include "src/libfred/registrable_object/contact/verification/info_check.hh"
#include "src/libfred/registrable_object/contact/verification/list_checks.hh"
#include "src/libfred/registrable_object/contact/verification/list_enum_objects.hh"
#include "src/libfred/registrable_object/contact/verification/update_check.hh"
#include "src/libfred/registrable_object/contact/verification/update_test.hh"
#include "src/util/log/context.hh"

namespace  Admin {
    void update_tests(
        LibFred::OperationContext&                  _ctx,
        const uuid&                             _check_handle,
        const vector<pair<string, string> >&    _test_statuses,
        unsigned long long                      _logd_request_id
    ) {
        Logging::Context log("update_tests");

        std::vector<std::string> allowed_statuses = LibFred::ContactCheckStatus::get_tests_updateable();
        // if current check status is not valid for tests change...
        if(
            std::find(
                allowed_statuses.begin(),
                allowed_statuses.end(),
                LibFred::InfoContactCheck(_check_handle).exec(_ctx)
                    .check_state_history.rbegin()->status_handle
            ) == allowed_statuses.end()
        ) {
            throw Admin::ExceptionCheckNotUpdateable();
        }

        try {
            for(vector<pair<string, string> >::const_iterator it = _test_statuses.begin();
                it != _test_statuses.end();
                ++it
            ) {
                LibFred::UpdateContactTest(
                    _check_handle,
                    it->first,
                    it->second,
                    _logd_request_id,
                    Optional<string>()
                ).exec(_ctx);
            }
        } catch(const LibFred::ExceptionUnknownCheckHandle&) {
            throw Admin::ExceptionUnknownCheckHandle();
        } catch(const LibFred::ExceptionUnknownTestHandle&) {
            throw Admin::ExceptionUnknownTestHandle();
        } catch(const LibFred::ExceptionUnknownCheckTestPair&) {
            throw Admin::ExceptionUnknownCheckTestPair();
        } catch(const LibFred::ExceptionUnknownTestStatusHandle&) {
            throw Admin::ExceptionUnknownTestStatusHandle();
        }
    }
}