#include "src/fredlib/public_request/public_request_lock_guard.h"

namespace Fred
{

namespace
{

PublicRequestId lock_by_identification(OperationContext &_ctx, const std::string &_identification);
PublicRequestId lock_by_id(OperationContext &_ctx, PublicRequestId _id);

}

PublicRequestLockGuardByIdentification::PublicRequestLockGuardByIdentification(
    OperationContext &_ctx,
    const std::string &_identification)
:   public_request_id_(lock_by_identification(_ctx, _identification))
{
}

PublicRequestLockGuardById::PublicRequestLockGuardById(
    OperationContext &_ctx,
    PublicRequestId _id)
:   public_request_id_(lock_by_id(_ctx, _id))
{
}

namespace
{

PublicRequestId lock_by_identification(OperationContext &_ctx, const std::string &_identification)
{
    //get lock to the end of transaction for given object
    const Database::Result res = _ctx.get_conn().exec_params(
        "SELECT prom.request_id,lock_public_request_lock(prom.object_id) "
        "FROM public_request_auth pra "
        "JOIN public_request_objects_map prom ON prom.request_id=pra.id "
        "WHERE pra.identification=$1::TEXT",
        Database::query_param_list(_identification));
    if (0 < res.size()) {
        const PublicRequestId public_request_id = static_cast< PublicRequestId >(res[0][0]);
        return public_request_id;
    }
    BOOST_THROW_EXCEPTION(PublicRequestLockGuardByIdentification::Exception().
                              set_public_request_doesnt_exist(_identification));
}

PublicRequestId lock_by_id(OperationContext &_ctx, PublicRequestId _id)
{
    //get lock to the end of transaction for given object
    const Database::Result res = _ctx.get_conn().exec_params(
        "SELECT lock_public_request_lock(object_id) "
        "FROM public_request_objects_map "
        "WHERE request_id=$1::BIGINT",
        Database::query_param_list(_id));
    if (0 < res.size()) {
        return _id;
    }
    BOOST_THROW_EXCEPTION(PublicRequestLockGuardById::Exception().
                              set_public_request_doesnt_exist(_id));
}

}

}//namespace Fred
