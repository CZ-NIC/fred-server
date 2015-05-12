#ifndef PUBLIC_REQUEST_LOCK_GUARD_H_F77FFFF66AD58705219B2B82AA1969FA//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PUBLIC_REQUEST_LOCK_GUARD_H_F77FFFF66AD58705219B2B82AA1969FA

#include "src/fredlib/object_state/typedefs.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"

namespace Fred {

class PublicRequestLockGuard
{
public:
    virtual PublicRequestId get_public_request_id()const = 0;
protected:
    virtual ~PublicRequestLockGuard() { }
};

class PublicRequestLockGuardByIdentification
{
public:
    DECLARE_EXCEPTION_DATA(public_request_doesnt_exist, std::string);/**< exception members for unknown identification*/
    struct Exception
    :   virtual Fred::OperationException,
        ExceptionData_public_request_doesnt_exist< Exception >
    {};
    PublicRequestLockGuardByIdentification(OperationContext &_ctx, const std::string &_identification);
    virtual ~PublicRequestLockGuardByIdentification() { }
private:
    PublicRequestId get_public_request_id()const { return public_request_id_; }
    const PublicRequestId public_request_id_;
};

class PublicRequestLockGuardById
{
public:
    DECLARE_EXCEPTION_DATA(public_request_doesnt_exist, PublicRequestId);/**< exception members for unknown id*/
    struct Exception
    :   virtual Fred::OperationException,
        ExceptionData_public_request_doesnt_exist< Exception >
    {};
    PublicRequestLockGuardById(OperationContext &_ctx, PublicRequestId _id);
    virtual ~PublicRequestLockGuardById() { }
private:
    PublicRequestId get_public_request_id()const { return public_request_id_; }
    const PublicRequestId public_request_id_;
};

}//namespace Fred

#endif//PUBLIC_REQUEST_LOCK_GUARD_H_F77FFFF66AD58705219B2B82AA1969FA
