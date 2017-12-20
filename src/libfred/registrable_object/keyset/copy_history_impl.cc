#include "src/libfred/registrable_object/keyset/copy_history_impl.hh"

namespace LibFred
{
    void copy_keyset_data_to_keyset_history_impl(
        LibFred::OperationContext& _ctx,
        const unsigned long long _keyset_id,
        const unsigned long long _historyid
    ) {
        const Database::Result res = _ctx.get_conn().exec_params(
            "INSERT INTO keyset_history( "
                "historyid,  id"
            ") "
            "SELECT "
                "$1::bigint, id "
            "FROM keyset "
            "WHERE id = $2::integer ",
            Database::query_param_list(_historyid)(_keyset_id)
        );

        if(res.rows_affected() != 1) {
            throw std::runtime_error("INSERT INTO keyset_history failed");
        }

        _ctx.get_conn().exec_params(
            "INSERT INTO dsrecord_history( "
                "historyid,  id, keysetid, keytag, alg, digesttype, digest, maxsiglife "
            ") "
            "SELECT "
                "$1::bigint, id, keysetid, keytag, alg, digesttype, digest, maxsiglife "
            "FROM dsrecord "
            "WHERE keysetid = $2::integer ",
            Database::query_param_list(_historyid)(_keyset_id)
        );

        _ctx.get_conn().exec_params(
            "INSERT INTO dnskey_history( "
                "historyid,  id, keysetid, flags, protocol, alg, key "
            ") "
            "SELECT "
                "$1::bigint, id, keysetid, flags, protocol, alg, key "
            "FROM dnskey "
            "WHERE keysetid = $2::integer ",
            Database::query_param_list(_historyid)(_keyset_id)
        );

        _ctx.get_conn().exec_params(
            "INSERT INTO keyset_contact_map_history( "
                "historyid,  keysetid, contactid "
            ") "
            "SELECT "
                "$1::bigint, keysetid, contactid "
            "FROM keyset_contact_map "
            "WHERE keysetid = $2::integer ",
            Database::query_param_list(_historyid)(_keyset_id)
        );

    }
}
