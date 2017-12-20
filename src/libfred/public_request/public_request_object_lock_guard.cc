#include "src/libfred/public_request/public_request_object_lock_guard.hh"

namespace LibFred {

PublicRequestsOfObjectLockGuardByObjectId::PublicRequestsOfObjectLockGuardByObjectId(
        OperationContext &_ctx,
        ObjectId _object_id)
:   ctx_(_ctx),
    object_id_(_object_id)
{
    //get lock to the end of transaction for given object
    if (0 < _ctx.get_conn().exec_params("SELECT lock_public_request_lock(id) "
                                        "FROM object "
                                        "WHERE id=$1::BIGINT",
                                        Database::query_param_list(_object_id)).size()) {
        return;
    }
    BOOST_THROW_EXCEPTION(Exception().set_object_doesnt_exist(_object_id));
}

} // namespace LibFred
