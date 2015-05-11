#include "src/fredlib/public_request/public_request_object_lock_guard.h"

namespace Fred {

PublicRequestObjectLockGuard::PublicRequestObjectLockGuard(OperationContext &_ctx, ObjectId _object_id)
:   object_id_(_object_id)
{
    //get lock to the end of transaction for given object
    if (0 < _ctx.get_conn().exec_params("SELECT lock_public_request_lock(id) "
                                        "FROM object "
                                        "WHERE id=$1::BIGINT",
                                        Database::query_param_list(object_id_)).size()) {
        return;
    }
    BOOST_THROW_EXCEPTION(Exception().set_object_doesnt_exist(object_id_));
}

}//namespace Fred
