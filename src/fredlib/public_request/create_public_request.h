#ifndef CREATE_PUBLIC_REQUEST_H_4C9FE3D9B8BB0233CD814C7F0E46D4C9//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CREATE_PUBLIC_REQUEST_H_4C9FE3D9B8BB0233CD814C7F0E46D4C9

#include "src/fredlib/public_request/public_request_type_iface.h"
#include "src/fredlib/public_request/public_request_object_lock_guard.h"
#include "util/optional_value.h"

namespace Fred {

class CreatePublicRequest
{
public:
    DECLARE_EXCEPTION_DATA(bad_type, std::string);/**< exception members for bad public request type*/
    struct Exception
    :   virtual Fred::OperationException,
        ExceptionData_bad_type< Exception >
    {};
    CreatePublicRequest(const PublicRequestTypeIface &_type);
    CreatePublicRequest(const PublicRequestTypeIface &_type,
                        const Optional< std::string > &_reason,
                        const Optional< std::string > &_email_to_answer,
                        const Optional< RegistrarId > &_registrar_id);
    ~CreatePublicRequest() { }
    CreatePublicRequest& set_reason(const std::string &_reason);
    CreatePublicRequest& set_email_to_answer(const std::string &_email);
    CreatePublicRequest& set_registrar_id(RegistrarId _id);
    PublicRequestId exec(OperationContext &_ctx, const PublicRequestObjectLockGuard &_locked_object)const;
private:
    const std::string type_;
    Optional< std::string > reason_;
    Optional< std::string > email_to_answer_;
    Optional< RegistrarId > registrar_id_;
};

}//namespace Fred

#endif//CREATE_PUBLIC_REQUEST_H_4C9FE3D9B8BB0233CD814C7F0E46D4C9
