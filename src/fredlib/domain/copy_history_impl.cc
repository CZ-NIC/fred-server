#include "src/fredlib/domain/copy_history_impl.h"

namespace Fred
{
    void copy_domain_data_to_domain_history_impl(
        Fred::OperationContext& _ctx,
        const unsigned long long _domain_id,
        const unsigned long long _historyid
    ) {
        const Database::Result res = _ctx.get_conn().exec_params(
            "INSERT INTO domain_history( "
                "historyid,  id, zone, registrant, nsset, exdate, keyset "
            ") "
            "SELECT "
                "$1::bigint, id, zone, registrant, nsset, exdate, keyset "
            "FROM domain "
            "WHERE id = $2::integer ",
            Database::query_param_list(_historyid)(_domain_id)
        );

        if(res.rows_affected() != 1) {
            throw std::runtime_error("INSERT INTO domain_history failed");
        }

        _ctx.get_conn().exec_params(
            "INSERT INTO domain_contact_map_history( "
                "historyid,  domainid, contactid, role "
            ") "
            "SELECT "
                "$1::bigint, domainid, contactid, role "
            "FROM domain_contact_map "
            "WHERE domainid = $2::integer ",
            Database::query_param_list(_historyid)(_domain_id)
        );

        _ctx.get_conn().exec_params(
            "INSERT INTO enumval_history( "
                "historyid,  domainid, exdate, publish "
            ") "
            "SELECT "
                "$1::bigint, domainid, exdate, publish "
            "FROM enumval "
            "WHERE domainid = $2::integer ",
            Database::query_param_list(_historyid)(_domain_id)
        );
    }
}
