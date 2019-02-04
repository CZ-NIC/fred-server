#include "src/backend/admin/contact/verification/related_records.hh"

#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/verification/info_check.hh"

#include "libfred/object_state/perform_object_state_request.hh"

#include <boost/assign/list_of.hpp>

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

/**
 * @param _is_mail true <=> it is mail; false <=> it is message
 */
static void add_related_communication_impl(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle,
        const std::set<unsigned long long>& _ids,
        bool _is_mail)
{
    using std::set;

    LibFred::OperationContextCreator ctx_info;

    LibFred::InfoContactCheckOutput check_info = LibFred::InfoContactCheck(_check_handle)
                                                 .exec(ctx_info);

    std::string collumn_name = (_is_mail) ? "mail_archive_id" : "message_archive_id";

    _ctx.get_conn().exec("SAVEPOINT related_communication_savepoint");
    // want to have as much commited as possible in case of exception in the middle of the processing"
    try
    {
        for (set<unsigned long long>::const_iterator it = _ids.begin();
             it != _ids.end();
             ++it
             )
        {
            _ctx.get_conn().exec("RELEASE SAVEPOINT related_communication_savepoint");
            _ctx.get_conn().exec("SAVEPOINT related_communication_savepoint");
            _ctx.get_conn().exec_params(
                    // clang-format off
                    "INSERT INTO contact_check_message_map"
                    "   (contact_check_id, "+ collumn_name +") "
                    "   VALUES( ( "
                    "       SELECT id "
                    "           FROM contact_check "
                    "           WHERE handle = $1::uuid "
                    "       ), "
                    "       $2::bigint"
                    "   )",
                    // clang-format on
                    Database::query_param_list(_check_handle)(*it));
        }
    }
    catch (...)
    {
        try
        {
            _ctx.get_conn().exec("ROLLBACK TO related_communication_savepoint");
        }
        catch (...)
        {
        }

        throw;
    }
}


void add_related_mail(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle,
        const std::set<unsigned long long>& _mail_archive_ids)
{
    add_related_communication_impl(
            _ctx,
            _check_handle,
            _mail_archive_ids,
            true);
}


void add_related_messages(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle,
        const std::set<unsigned long long>& _message_archive_ids)
{
    add_related_communication_impl(
            _ctx,
            _check_handle,
            _message_archive_ids,
            false);
}


void add_related_object_state_requests(
        LibFred::OperationContext& _ctx,
        const uuid& _check_handle,
        const std::set<unsigned long long>& _request_ids)
{
    using std::set;

    LibFred::InfoContactCheckOutput check_info =
            LibFred::InfoContactCheck(_check_handle).exec(_ctx);

    const unsigned long long contact_id =
        LibFred::InfoContactHistoryByHistoryid(check_info.contact_history_id).exec(_ctx)
        .info_contact_data.id;

    _ctx.get_conn().exec("SAVEPOINT related_object_request_savepoint");
    // want to have as much commited as possible in case of exception in the middle of the processing"
    try
    {
        for (set<unsigned long long>::const_iterator it = _request_ids.begin();
             it != _request_ids.end();
             ++it
             )
        {
            _ctx.get_conn().exec("RELEASE SAVEPOINT related_object_request_savepoint");
            _ctx.get_conn().exec("SAVEPOINT related_object_request_savepoint");
            _ctx.get_conn().exec_params(
                    // clang-format off
                    "INSERT INTO contact_check_object_state_request_map "
                    "   (contact_check_id, object_state_request_id) "
                    "   VALUES( ( "
                    "       SELECT id "
                    "           FROM contact_check "
                    "           WHERE handle = $1::uuid "
                    "       ), "
                    "       $2::bigint"
                    "   )",
                    // clang-format on
                    Database::query_param_list(_check_handle)(*it));
        }
    }
    catch (...)
    {
        try
        {
            _ctx.get_conn().exec("ROLLBACK TO related_object_request_savepoint");
            LibFred::PerformObjectStateRequest(contact_id).exec(_ctx);
        }
        catch (...)
        {
        }

        throw;
    }
}


vector<related_message> get_related_messages(
        LibFred::OperationContext& _ctx,
        unsigned long long _contact_id,
        const std::string&      _output_timezone)
{
    static const std::map<unsigned, std::string> mail_status_names =
        boost::assign::map_list_of
            // clang-format off
            (0, "sent")
            (1, "ready")
            (2, "waiting_confirmation")
            (3, "no_processing")
            (4, "send_failed");
            // clang-format on

    Database::Result related_res = _ctx.get_conn().exec_params(
            // clang-format off
            "WITH check_id AS ( "
                "SELECT c_ch.id "
                    "FROM contact_history AS c_h "
                        "JOIN contact_check AS c_ch ON c_h.historyid = c_ch.contact_history_id "
                    "WHERE c_h.id = $1::bigint "
            ") ( "
                "SELECT "
                    "c_ch_m_map.mail_archive_id  AS id_, "
                    "'email'                     AS comm_type_, "
                    "m_t.name                    AS content_type_, "
                    "m_a.crdate                  "
                        "AT TIME ZONE 'utc' "                                   /* conversion from 'utc' ... */
                        "AT TIME ZONE $2::text "
                                                 "AS created_, "
                    "m_a.moddate "
                        "AT TIME ZONE 'utc' "                                   /* conversion from 'utc' ... */
                        "AT TIME ZONE $2::text "
                                                "AS updated_, "
                    "m_a.status                  AS status_id_, "
                    "''                          AS status_name_ "
                "FROM "
                    "check_id "
                    "JOIN contact_check_message_map  AS c_ch_m_map   ON check_id.id = c_ch_m_map.contact_check_id "
                    "JOIN mail_archive               AS m_a          ON c_ch_m_map.mail_archive_id = m_a.id "
                    "JOIN mail_type                  AS m_t          ON m_a.mail_type_id = m_t.id "
                "UNION "
                "SELECT "
                    "c_ch_m_map.message_archive_id   AS id_, "
                    "c_t.type                        AS comm_type_, "
                    "m_t.type                        AS content_type_, "
                    "m_a.crdate "
                        "AT TIME ZONE 'utc' "                                   /* conversion from 'utc' ... */
                        "AT TIME ZONE $2::text "
                                                    "AS created_, "
                    "m_a.moddate "
                        "AT TIME ZONE 'utc' "                                   /* conversion from 'utc' ... */
                        "AT TIME ZONE $2::text "
                                                    "AS updated_, "
                    "m_a.status_id                   AS status_id_, "
                    "enum_s_st.status_name           AS status_name_ "
                "FROM "
                    "check_id "
                    "JOIN contact_check_message_map  AS c_ch_m_map   ON check_id.id = c_ch_m_map.contact_check_id "
                    "JOIN message_archive            AS m_a          ON c_ch_m_map.message_archive_id = m_a.id "
                    "JOIN comm_type                  AS c_t          ON m_a.comm_type_id = c_t.id "
                    "JOIN message_type               AS m_t          ON m_a.message_type_id = m_t.id "
                    "JOIN enum_send_status           AS enum_s_st    ON enum_s_st.id = m_a.status_id "
            ") ",
            // clang-format on
            Database::query_param_list(
                    _contact_id)(_output_timezone));

    vector<related_message> result;

    for (Database::Result::Iterator it = related_res.begin(); it != related_res.end(); ++it)
    {
        using boost::posix_time::ptime;
        using boost::posix_time::time_from_string;

        result.push_back(
                related_message(
                        static_cast<unsigned long long>((*it)["id_"]),
                        static_cast<std::string>((*it)["comm_type_"]),
                        static_cast<std::string>((*it)["content_type_"]),
                        static_cast<ptime>(time_from_string(static_cast<string>((*it)["created_"]))),
                        ((*it)["updated_"].isnull())
                            ? Nullable<ptime>()
                            : Nullable<ptime>(time_from_string(static_cast<string>((*it)["updated_"]))),
                        static_cast<unsigned>((*it)["status_id_"]),
                        (static_cast<std::string>((*it)["comm_type_"]) == "email")
                            ? mail_status_names.at(static_cast<unsigned>((*it)["status_id_"]))
                            : static_cast<string>((*it)["status_name_"])));
    }

    return result;
}

} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred
