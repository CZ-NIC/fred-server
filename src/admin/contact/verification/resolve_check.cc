#include "src/admin/contact/verification/resolve_check.h"
#include "src/admin/contact/verification/related_records_impl.h"
#include "src/admin/contact/verification/contact_states/enum.h"
#include "src/admin/contact/verification/contact_states/delete_all.h"
#include "src/fredlib/contact/verification/enum_testsuite_handle.h"
#include "src/fredlib/contact/verification/enum_check_status.h"
#include "src/fredlib/contact/verification/update_check.h"
#include "src/fredlib/contact/verification/info_check.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/perform_object_state_request.h"

#include "util/log/context.h"

#include <set>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

namespace  Admin {
    resolve_check::resolve_check(
        const uuid&                     _check_handle,
        const std::string&              _status_handle,
        Optional<unsigned long long>    _logd_request_id
    ) :
        check_handle_(_check_handle),
        status_handle_(_status_handle),
        logd_request_id_(_logd_request_id)
    { }

    resolve_check& resolve_check::set_logd_request_id(Optional<unsigned long long> _logd_request_id) {
        logd_request_id_ = _logd_request_id;

        return *this;
    }

    void resolve_check::exec(Fred::OperationContext& _ctx) {
        Logging::Context log("resolve_check::exec");

        std::vector<std::string> allowed_statuses = Fred::ContactCheckStatus::get_resolution_awaiting();
        // if current check status is not valid for resolution...
        if(
            std::find(
                allowed_statuses.begin(),
                allowed_statuses.end(),
                Fred::InfoContactCheck(check_handle_).exec(_ctx)
                    .check_state_history.rbegin()->status_handle
            ) == allowed_statuses.end()
        ) {
            throw Admin::ExceptionCheckNotUpdateable();
        }

        try {
            Fred::UpdateContactCheck(
                check_handle_,
                status_handle_,
                logd_request_id_
            ).exec(_ctx);

            Fred::InfoContactCheckOutput check_info = Fred::InfoContactCheck(
                check_handle_
            ).exec(_ctx);


            if(check_info.testsuite_handle == Fred::TestsuiteHandle::AUTOMATIC) {
                postprocess_automatic_check(_ctx, check_handle_);
            } else if(check_info.testsuite_handle == Fred::TestsuiteHandle::MANUAL) {
                postprocess_manual_check(_ctx, check_handle_);
            } else if(check_info.testsuite_handle == Fred::TestsuiteHandle::THANK_YOU) {
                postprocess_thank_you_check(_ctx, check_handle_);
            }
        } catch (const Fred::ExceptionUnknownCheckHandle& ) {
            throw Admin::ExceptionUnknownCheckHandle();

        } catch (const Fred::ExceptionUnknownCheckStatusHandle& ) {
            throw Admin::ExceptionUnknownCheckStatusHandle();
        }
    }

    void resolve_check::postprocess_automatic_check(
        Fred::OperationContext& _ctx,
        const uuid& _check_handle
    ) {
        Fred::InfoContactCheckOutput check_info = Fred::InfoContactCheck(
            _check_handle
        ).exec(_ctx);

        Fred::InfoContactOutput contact_info = Fred::InfoContactHistoryByHistoryid(
            check_info.contact_history_id
        ).exec(_ctx);

        AdminContactVerificationObjectStates::delete_all(_ctx, contact_info.info_contact_data.id);

        const std::string& new_handle = check_info.check_state_history.rbegin()->status_handle;
        if( new_handle == Fred::ContactCheckStatus::OK ) {

            std::set<std::string> status;
            status.insert(Admin::AdminContactVerificationObjectStates::CONTACT_PASSED_MANUAL_VERIFICATION);

            std::set<unsigned long long> state_request_ids;
            state_request_ids.insert(
                Fred::CreateObjectStateRequestId(
                    contact_info.info_contact_data.id,
                    status
                ).exec(_ctx)
                .second
            );

            Admin::add_related_object_state_requests(_ctx, _check_handle, state_request_ids);
        }

        Fred::PerformObjectStateRequest(contact_info.info_contact_data.id).exec(_ctx);
    }

    void resolve_check::postprocess_manual_check(
        Fred::OperationContext& _ctx,
        const uuid& _check_handle
    ) {

        Fred::InfoContactCheckOutput check_info = Fred::InfoContactCheck(
            _check_handle
        ).exec(_ctx);

        Fred::InfoContactOutput contact_info = Fred::InfoContactHistoryByHistoryid(
            check_info.contact_history_id
        ).exec(_ctx);

        AdminContactVerificationObjectStates::delete_all(_ctx, contact_info.info_contact_data.id);

        const std::string& new_handle = check_info.check_state_history.rbegin()->status_handle;
        if( new_handle == Fred::ContactCheckStatus::OK
            ||
            new_handle == Fred::ContactCheckStatus::FAIL
        ) {
            using namespace Admin::AdminContactVerificationObjectStates;

            std::set<std::string> status;

            if(new_handle ==      Fred::ContactCheckStatus::OK) { status.insert(   CONTACT_PASSED_MANUAL_VERIFICATION); }
            else if(new_handle == Fred::ContactCheckStatus::FAIL) { status.insert( CONTACT_FAILED_MANUAL_VERIFICATION); }

            std::set<unsigned long long> state_request_ids;
            state_request_ids.insert(
                Fred::CreateObjectStateRequestId(
                    contact_info.info_contact_data.id,
                    status
                ).exec(_ctx)
                .second
            );

            Admin::add_related_object_state_requests(_ctx, _check_handle, state_request_ids);
        }

        Fred::PerformObjectStateRequest(contact_info.info_contact_data.id).exec(_ctx);
    }

    void resolve_check::postprocess_thank_you_check(
        Fred::OperationContext& _ctx,
        const uuid& _check_handle
    ) {
        // it is exactly the same
        postprocess_manual_check(_ctx, _check_handle);
    }
}
