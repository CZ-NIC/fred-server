#ifndef PUBLIC_REQUEST_OBJECT_LOCK_GUARD_H_A5806AB6CD121D576783659ADFF6343F//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PUBLIC_REQUEST_OBJECT_LOCK_GUARD_H_A5806AB6CD121D576783659ADFF6343F

#include "src/fredlib/object_state/typedefs.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"

namespace Fred {

class PublicRequestObjectLockGuard
{
public:
    DECLARE_EXCEPTION_DATA(object_doesnt_exist, ObjectId);/**< exception members for bad object_id*/
    struct Exception
    :   virtual Fred::OperationException,
        ExceptionData_object_doesnt_exist< Exception >
    {};
    PublicRequestObjectLockGuard(OperationContext &_ctx, ObjectId _object_id);
    ObjectId get_object_id()const { return object_id_; }
private:
    const ObjectId object_id_;
};

}//namespace Fred

#endif//PUBLIC_REQUEST_OBJECT_LOCK_GUARD_H_A5806AB6CD121D576783659ADFF6343F
