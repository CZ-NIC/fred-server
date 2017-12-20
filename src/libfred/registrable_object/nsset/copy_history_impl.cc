#include "src/libfred/registrable_object/nsset/copy_history_impl.hh"

namespace LibFred
{
    void copy_nsset_data_to_nsset_history_impl(
        LibFred::OperationContext& _ctx,
        const unsigned long long _nsset_id,
        const unsigned long long _historyid
    ) {
        const Database::Result res = _ctx.get_conn().exec_params(
            "INSERT INTO nsset_history( "
                "historyid,  id, checklevel "
            ") "
            "SELECT "
                "$1::bigint, id, checklevel "
            "FROM nsset "
            "WHERE id = $2::integer",
            Database::query_param_list(_historyid)(_nsset_id)
        );

        if(res.rows_affected() != 1) {
            throw std::runtime_error("INSERT INTO nsset_history failed");
        }

        _ctx.get_conn().exec_params(
            "INSERT INTO host_history( "
                "historyid,  id, nssetid, fqdn "
            ") "
            "SELECT "
                "$1::bigint, id, nssetid, fqdn "
            "FROM host "
            "WHERE nssetid = $2::integer ",
            Database::query_param_list(_historyid)(_nsset_id)
        );

        _ctx.get_conn().exec_params(
            "INSERT INTO host_ipaddr_map_history( "
                "historyid,  id, hostid, nssetid, ipaddr "
            ") "
            "SELECT "
                "$1::bigint, id, hostid, nssetid, ipaddr "
            "FROM host_ipaddr_map "
            "WHERE nssetid = $2::integer ",
            Database::query_param_list(_historyid)(_nsset_id)
        );

        _ctx.get_conn().exec_params(
            "INSERT INTO nsset_contact_map_history( "
                "historyid,  nssetid, contactid "
            ") "
            "SELECT "
                "$1::bigint, nssetid, contactid "
            "FROM nsset_contact_map "
            "WHERE nssetid = $2::integer ",
            Database::query_param_list(_historyid)(_nsset_id)
        );
    }
}
