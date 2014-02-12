#include "src/admin/contact/verification/resolve_check.h"
#include "src/admin/contact/verification/related_records_impl.h"
#include "src/fredlib/contact/verification/enum_testsuite_handle.h"
#include "src/fredlib/contact/verification/enum_check_status.h"
#include "src/fredlib/contact/verification/update_check.h"
#include "src/fredlib/contact/verification/info_check.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/cancel_object_state_request_id.h"

#include <set>
#include <boost/assign/list_of.hpp>

namespace  Admin {
    resolve_check::resolve_check(
        const std::string&              _check_handle,
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
        }
    }

    void resolve_check::postprocess_automatic_check(
        Fred::OperationContext& _ctx,
        const std::string& _check_handle
    ) {
        // in case of need feel free to express yourself...
    }

    void resolve_check::postprocess_manual_check(
        Fred::OperationContext& _ctx,
        const std::string& _check_handle
    ) {

        Fred::InfoContactCheckOutput check_info = Fred::InfoContactCheck(
            _check_handle
        ).exec(_ctx);

        Fred::InfoContactOutput contact_info = Fred::HistoryInfoContactByHistoryid(
            check_info.contact_history_id
        ).exec(_ctx);

        _ctx.get_conn().exec("SAVEPOINT state_savepoint");

        // cancel one state at a time because when exception is thrown, all changes would be ROLLBACKed
        try {
            std::set<std::string> object_states_to_erase =
                boost::assign::list_of("contactInManualVerification");
            Fred::CancelObjectStateRequestId(
                contact_info.info_contact_data.id,
                object_states_to_erase
            ).exec(_ctx);
            _ctx.get_conn().exec("RELEASE SAVEPOINT state_savepoint");
            _ctx.get_conn().exec("SAVEPOINT state_savepoint");
        } catch(Fred::CancelObjectStateRequestId::Exception& e) {
            // in case it throws from with unknown cause
            if(e.is_set_state_not_found() == false) {
                throw;
            } else {
                _ctx.get_conn().exec("ROLLBACK TO state_savepoint");
            }
        }
        try {
            std::set<std::string> object_states_to_erase =
                boost::assign::list_of("manuallyVerifiedContact");

            Fred::CancelObjectStateRequestId(
                contact_info.info_contact_data.id,
                object_states_to_erase
            ).exec(_ctx);

            _ctx.get_conn().exec("RELEASE SAVEPOINT state_savepoint");
            _ctx.get_conn().exec("SAVEPOINT state_savepoint");
        } catch(Fred::CancelObjectStateRequestId::Exception& e) {
            // in case it throws from with unknown cause
            if(e.is_set_state_not_found() == false) {
                throw;
            } else {
                _ctx.get_conn().exec("ROLLBACK TO state_savepoint");
            }
        }

        if(check_info.check_state_history.rbegin()->status_handle == Fred::ContactCheckStatus::OK) {
            std::set<std::string> status;
            status.insert("manuallyVerifiedContact");

            std::set<unsigned long long> state_request_ids;
            state_request_ids.insert(
                Fred::CreateObjectStateRequestId(
                    contact_info.info_contact_data.id,
                    status
                ).exec(_ctx)
                .second
            );

            Admin::add_related_object_state_requests(_ctx, check_handle_, state_request_ids);
        }
    }
}
