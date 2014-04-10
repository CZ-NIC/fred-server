#include "src/admin/contact/verification/related_records_impl.h"
#include "src/fredlib/contact/verification/info_check.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/object_state/create_object_state_request.h"

namespace  Admin {

    /**
     * @param _is_mail true <=> it is mail; false <=> it is message
     */
    static void add_related_communication_impl(
        Fred::OperationContext&             _ctx,
        const uuid&                         _check_handle,
        const std::set<unsigned long long>& _ids,
        bool                                _is_mail
    ){
        using std::set;

        Fred::OperationContext ctx_info;

        Fred::InfoContactCheckOutput check_info = Fred::InfoContactCheck(_check_handle)
            .exec(ctx_info);

        std::string collumn_name = (_is_mail) ? "mail_archive_id" : "message_archive_id";

        _ctx.get_conn().exec("SAVEPOINT related_communication_savepoint");
        // want to have as much commited as possible in case of exception in the middle of the processing"
        try {
            for(set<unsigned long long>::const_iterator it = _ids.begin();
                it != _ids.end();
                ++it
            ) {
                _ctx.get_conn().exec("RELEASE SAVEPOINT related_communication_savepoint");
                _ctx.get_conn().exec("SAVEPOINT related_communication_savepoint");
                _ctx.get_conn().exec_params(
                    "INSERT INTO contact_check_message_map"
                    "   (contact_check_id, "+ collumn_name +") "
                    "   VALUES( ( "
                    "       SELECT id "
                    "           FROM contact_check "
                    "           WHERE handle = $1::uuid "
                    "       ), "
                    "       $2::bigint"
                    "   )",
                    Database::query_param_list
                        (_check_handle)
                        (*it)
                );
            }
        } catch(...) {
            try {
                _ctx.get_conn().exec("ROLLBACK TO related_communication_savepoint");
            } catch(...) { }

            throw;
        }
    }

    void add_related_mail(
        Fred::OperationContext&             _ctx,
        const uuid&                         _check_handle,
        const std::set<unsigned long long>& _mail_archive_ids
    ) {
        add_related_communication_impl(_ctx, _check_handle, _mail_archive_ids, true);
    }

    void add_related_messages(
        Fred::OperationContext&             _ctx,
        const uuid&                         _check_handle,
        const std::set<unsigned long long>& _message_archive_ids
    ) {
        add_related_communication_impl(_ctx, _check_handle, _message_archive_ids, false);
    }

    void add_related_object_state_requests(
        Fred::OperationContext&             _ctx,
        const uuid&                         _check_handle,
        const std::set<unsigned long long>& _request_ids
    ) {
        using std::set;

        Fred::InfoContactCheckOutput check_info = Fred::InfoContactCheck(_check_handle)
            .exec(_ctx);

        const unsigned long long contact_id =
        Fred::InfoContactHistoryByHistoryid(
            check_info.contact_history_id
        ).exec(_ctx)
        .info_contact_data.id;

        _ctx.get_conn().exec("SAVEPOINT related_object_request_savepoint");
        // want to have as much commited as possible in case of exception in the middle of the processing"
        try {
            for(set<unsigned long long>::const_iterator it = _request_ids.begin();
                it != _request_ids.end();
                ++it
            ) {
                _ctx.get_conn().exec("RELEASE SAVEPOINT related_object_request_savepoint");
                _ctx.get_conn().exec("SAVEPOINT related_object_request_savepoint");
                _ctx.get_conn().exec_params(
                    "INSERT INTO contact_check_object_state_request_map "
                    "   (contact_check_id, object_state_request_id) "
                    "   VALUES( ( "
                    "       SELECT id "
                    "           FROM contact_check "
                    "           WHERE handle = $1::uuid "
                    "       ), "
                    "       $2::bigint"
                    "   )",
                    Database::query_param_list
                        (_check_handle)
                        (*it)
                );
            }
        } catch(...) {
            try {
                _ctx.get_conn().exec("ROLLBACK TO related_object_request_savepoint");
                Fred::PerformObjectStateRequest(contact_id).exec(_ctx);
            } catch(...) { }

            throw;
        }
    }
}
