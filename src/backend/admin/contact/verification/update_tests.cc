#include "src/backend/admin/contact/verification/update_tests.hh"

#include "libfred/registrable_object/contact/verification/create_check.hh"
#include "libfred/registrable_object/contact/verification/create_test.hh"
#include "libfred/registrable_object/contact/verification/enum_check_status.hh"
#include "libfred/registrable_object/contact/verification/enum_test_status.hh"
#include "src/deprecated/libfred/registrable_object/contact/verification/enum_testsuite_handle.hh"
#include "libfred/registrable_object/contact/verification/exceptions.hh"
#include "libfred/registrable_object/contact/verification/info_check.hh"
#include "libfred/registrable_object/contact/verification/list_checks.hh"
#include "libfred/registrable_object/contact/verification/list_enum_objects.hh"
#include "libfred/registrable_object/contact/verification/update_check.hh"
#include "libfred/registrable_object/contact/verification/update_test.hh"
#include "util/log/context.hh"

#include <utility>
#include <vector>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

void update_tests(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle,
        const vector<pair<string, string> >& _test_statuses,
        unsigned long long _logd_request_id)
{
    Logging::Context log("update_tests");

    std::vector<std::string> allowed_statuses = LibFred::ContactCheckStatus::get_tests_updateable();
    // if current check status is not valid for tests change...
    if (
        std::find(
                allowed_statuses.begin(),
                allowed_statuses.end(),
                LibFred::InfoContactCheck(_check_handle).exec(_ctx)
                .check_state_history.rbegin()->status_handle) == allowed_statuses.end()
        )
    {
        throw ExceptionCheckNotUpdateable();
    }

    try
    {
        for (vector<pair<string, string> >::const_iterator it = _test_statuses.begin();
             it != _test_statuses.end();
             ++it
             )
        {
            LibFred::UpdateContactTest(
                    _check_handle,
                    it->first,
                    it->second,
                    _logd_request_id,
                    Optional<string>()).exec(_ctx);
        }
    }
    catch (const LibFred::ExceptionUnknownCheckHandle&)
    {
        throw ExceptionUnknownCheckHandle();
    }
    catch (const LibFred::ExceptionUnknownTestHandle&)
    {
        throw ExceptionUnknownTestHandle();
    }
    catch (const LibFred::ExceptionUnknownCheckTestPair&)
    {
        throw ExceptionUnknownCheckTestPair();
    }
    catch (const LibFred::ExceptionUnknownTestStatusHandle&)
    {
        throw ExceptionUnknownTestStatusHandle();
    }
}

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
