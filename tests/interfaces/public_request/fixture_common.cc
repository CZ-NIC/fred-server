#include "tests/interfaces/public_request/fixture_common.h"
#include "src/fredlib/db_settings.h"

Database::Result get_db_public_request(
    const Fred::OperationContext& ctx,
    const unsigned long long id,
    const unsigned type,
    const unsigned status)
{
    return ctx.get_conn().exec_params(
            "SELECT * FROM public_request "
            "WHERE id=$1::bigint "
                "AND request_type=$2::smallint "
                "AND status=$3::smallint "
                "AND reason IS NULL "
                "AND email_to_answer IS NULL "
                "AND registrar_id IS NULL ",
            Database::query_param_list(id)(type)(status));
}

Database::Result get_db_public_request(
    const Fred::OperationContext& ctx,
    const unsigned long long id,
    const unsigned type,
    const unsigned status,
    const std::string& reason)
{
    return ctx.get_conn().exec_params(
            "SELECT * FROM public_request "
            "WHERE id=$1::bigint "
                "AND request_type=$2::smallint "
                "AND status=$3::smallint "
                "AND reason=$4::text "
                "AND email_to_answer IS NULL "
                "AND registrar_id IS NULL ",
            Database::query_param_list(id)(type)(status)(reason));
}

Database::Result get_db_public_request(
    const Fred::OperationContext& ctx,
    const unsigned long long id,
    const unsigned type,
    const unsigned status,
    const std::string& reason,
    const std::string& email_to_answer)
{
    return ctx.get_conn().exec_params(
            "SELECT * FROM public_request "
            "WHERE id=$1::bigint "
                "AND request_type=$2::smallint "
                "AND status=$3::smallint "
                "AND reason=$4::text "
                "AND email_to_answer=$5::text "
                "AND registrar_id IS NULL ",
            Database::query_param_list(id)(type)(status)(reason)(email_to_answer));
}
