#ifndef CREATE_PUBLIC_REQUEST_H_4C9FE3D9B8BB0233CD814C7F0E46D4C9//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CREATE_PUBLIC_REQUEST_H_4C9FE3D9B8BB0233CD814C7F0E46D4C9

#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"

namespace Fred {

class PublicRequestTypeIface
{
public:
    virtual std::string get_public_request_type()const = 0;
protected:
    virtual ~PublicRequestTypeIface() { }
};

namespace PublicRequest
{

class AuthinfoAutoRifType:public PublicRequestTypeIface
{
public:
    ~AuthinfoAutoRifType() { }
protected:
    std::string get_public_request_type()const { return "authinfo_auto_rif"; }
};

}//Fred::PublicRequest

typedef ::uint64_t ObjectId;
typedef ObjectId PublicRequestId;
typedef ObjectId RegistrarId;

class PublicRequestObjectLockGuard
{
public:
    PublicRequestObjectLockGuard(OperationContext &_ctx, ObjectId _object_id);
    ObjectId get_object_id()const { return object_id_; }
private:
    const ObjectId object_id_;
};

PublicRequestId create_public_request(
    OperationContext &_ctx,
    const PublicRequestObjectLockGuard &_locked_object,
    const PublicRequestTypeIface &_type,
    const Optional< std::string > &_reason,
    const Optional< std::string > &_email_to_answer,
    const Optional< RegistrarId > &_registrar_id);

enum { PUBLIC_REQUEST_AUTH_IDENTIFICATION_LENGTH = 32 };

struct CreatePublicRequestAuthResult
{
    CreatePublicRequestAuthResult() { }
    CreatePublicRequestAuthResult(const CreatePublicRequestAuthResult &_src)
    :   public_request_id(_src.public_request_id),
        identification(_src.identification)
    { }
    CreatePublicRequestAuthResult& operator=(const CreatePublicRequestAuthResult &_src)
    {
        public_request_id = _src.public_request_id;
        identification = _src.identification;
        return *this;
    }
    PublicRequestId public_request_id;
    std::string identification;
    std::string password;
};

CreatePublicRequestAuthResult create_public_request_auth(
    OperationContext &_ctx,
    const PublicRequestObjectLockGuard &_locked_object,
    const PublicRequestTypeIface &_type,
    const std::string &_password,
    const Optional< std::string > &_reason,
    const Optional< std::string > &_email_to_answer,
    const Optional< RegistrarId > &_registrar_id);

}//namespace Fred

#endif//CREATE_PUBLIC_REQUEST_H_4C9FE3D9B8BB0233CD814C7F0E46D4C9
