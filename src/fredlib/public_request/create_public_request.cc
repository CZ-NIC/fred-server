#include "src/fredlib/public_request/create_public_request.h"

namespace Fred {

PublicRequestObjectLockGuard::PublicRequestObjectLockGuard(OperationContext &_ctx, ObjectId _object_id)
:   object_id_(_object_id)
{
    //get lock to the end of transaction for given object
    _ctx.get_conn().exec_params("SELECT lock_public_request_lock($1::BIGINT)",
        Database::query_param_list(object_id_));
}

PublicRequestId create_public_request(
    OperationContext &_ctx,
    const PublicRequestObjectLockGuard &_locked_object,
    const PublicRequestTypeIface &_type,
    const Optional< std::string > &_reason,
    const Optional< std::string > &_email_to_answer,
    const Optional< RegistrarId > &_registrar_id)
{
    try {
        Database::query_param_list params(_type.get_public_request_type());
        params(_locked_object.get_object_id())
              (_reason.isset() ? _reason.get_value() : Database::QPNull)
              (_email_to_answer.isset() ? _email_to_answer.get_value() : Database::QPNull);
        if (_registrar_id.isset()) {
            params(_registrar_id.get_value());
        }
        else {
            params(Database::QPNull);
        };
        const Database::Result res = _ctx.get_conn().exec_params(
            "WITH request AS ("
                "INSERT INTO public_request "
                    "(request_type,status,resolve_time,reason,email_to_answer,answer_email_id,registrar_id,"
                     "create_request_id,resolve_request_id) "
                "VALUES ((SELECT id FROM enum_public_request_type WHERE name=$1::TEXT),"
                        "(SELECT id FROM enum_public_request_status WHERE name='new'),"
                        "NULL,$3::TEXT,$4::TEXT,NULL,$5::BIGINT,NULL,NULL) "
                "RETURNING id) "
            "INSERT INTO public_request_objects_map (request_id,object_id) "
                "SELECT id,$2::BIGINT FROM request "
            "RETURNING request_id", params);
        const PublicRequestId public_request_id = static_cast< PublicRequestId >(res[0][0]);
        return public_request_id;
    }
    catch (const std::runtime_error &e) {
        throw;
    }
}

}//namespace Fred
