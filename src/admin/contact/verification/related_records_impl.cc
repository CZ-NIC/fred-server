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

    vector< tuple< unsigned long long, string, string > > get_related_messages(
        Fred::OperationContext& _ctx,
        const uuid&             _check_handle
    ) {
        Database::Result related_res = _ctx.get_conn().exec_params(
            "WITH check_id AS ( "
            "SELECT id "
                "FROM contact_check "
                "WHERE handle = $1::uuid "
            ") ( "
                "SELECT "
                    "c_ch_m_map.mail_archive_id  AS id_, "
                    "'email'                     AS comm_type_, "
                    "m_t.name                    AS content_type_ "
                "FROM "
                    "check_id "
                    "JOIN contact_check_message_map  AS c_ch_m_map   ON check_id.id = c_ch_m_map.contact_check_id "
                    "JOIN mail_archive               AS m_a          ON c_ch_m_map.mail_archive_id = m_a.id "
                    "JOIN mail_type                  AS m_t          ON m_a.mailtype = m_t.id "
                "UNION "
                "SELECT "
                    "c_ch_m_map.message_archive_id   AS id_, "
                    "c_t.type                        AS comm_type_, "
                    "m_t.type                        AS content_type_ "
                "FROM "
                    "check_id "
                    "JOIN contact_check_message_map  AS c_ch_m_map   ON check_id.id = c_ch_m_map.contact_check_id "
                    "JOIN message_archive            AS m_a          ON c_ch_m_map.message_archive_id = m_a.id "
                    "JOIN comm_type                  AS c_t          ON m_a.comm_type_id = c_t.id "
                    "JOIN message_type               AS m_t          ON m_a.message_type_id = m_t.id "
            ") ",
            Database::query_param_list(_check_handle)
        );

        vector< tuple< unsigned long long, string, string > > result;

        for(Database::Result::Iterator it = related_res.begin(); it != related_res.end(); ++it) {
            result.push_back(boost::make_tuple(
                (*it)["id_"],
                (*it)["comm_type_"],
                (*it)["content_type_"]
            ));
        }

        return result;
    }
}
