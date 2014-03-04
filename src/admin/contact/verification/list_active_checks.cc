#include "src/admin/contact/verification/list_active_checks.h"

#include <vector>
#include <boost/assign/list_of.hpp>

namespace  Admin {
    std::vector<Fred::ListChecksItem> list_active_checks(const Optional<std::string>& _testsuite_handle) {
        std::vector<Fred::ListChecksItem> result;

        std::vector<std::string> awaiting_statuses = Fred::ContactCheckStatus::get_all();

        Fred::OperationContext ctx;
        std::vector<Fred::ListChecksItem> temp_result;
        for(std::vector<std::string>::const_iterator it = awaiting_statuses.begin();
            it != awaiting_statuses.end();
            ++it
        ) {
            temp_result = Fred::ListContactChecks()
                .set_status_handle( *it )
                .exec(ctx);

            result.reserve( result.size() + temp_result.size() );
            result.insert( result.end(), temp_result.begin(), temp_result.end() );
        }

        return result;
    }
}
