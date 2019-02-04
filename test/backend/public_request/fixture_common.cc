#include "test/backend/public_request/fixture_common.hh"
#include "libfred/db_settings.hh"

Database::Result get_db_public_request(
    const ::LibFred::OperationContext& ctx,
    const unsigned long long id,
    const unsigned type,
    const unsigned status)
{
    return ctx.get_conn().exec_params(
            "SELECT id,request_type,create_time,status,resolve_time,reason,email_to_answer,"
                   "answer_email_id,registrar_id,create_request_id,resolve_request_id "
            "FROM public_request "
            "WHERE id=$1::bigint AND "
                  "request_type=$2::smallint AND "
                  "status=$3::smallint AND "
                  "email_to_answer IS NULL AND "
                  "registrar_id IS NULL ",
            Database::query_param_list(id)(type)(status));
}

Database::Result get_db_public_request(
    const ::LibFred::OperationContext& ctx,
    const unsigned long long id,
    const unsigned type,
    const unsigned status,
    const std::string& email_to_answer)
{
    return ctx.get_conn().exec_params(
            "SELECT id,request_type,create_time,status,resolve_time,reason,email_to_answer,"
                   "answer_email_id,registrar_id,create_request_id,resolve_request_id "
            "FROM public_request "
            "WHERE id=$1::bigint AND "
                  "request_type=$2::smallint AND "
                  "status=$3::smallint AND "
                  "email_to_answer=$4::text AND "
                  "registrar_id IS NULL ",
            Database::query_param_list(id)(type)(status)(email_to_answer));
}
