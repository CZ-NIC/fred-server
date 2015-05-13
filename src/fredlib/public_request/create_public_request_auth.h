#ifndef CREATE_PUBLIC_REQUEST_AUTH_H_D034201C95EC2E9FF787F4CBF17E7802//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CREATE_PUBLIC_REQUEST_AUTH_H_D034201C95EC2E9FF787F4CBF17E7802

#include "src/fredlib/public_request/public_request_type_iface.h"
#include "src/fredlib/public_request/public_request_object_lock_guard.h"
#include "util/optional_value.h"

namespace Fred {

enum { PUBLIC_REQUEST_AUTH_IDENTIFICATION_LENGTH = 32 };

class CreatePublicRequestAuth
{
public:
    DECLARE_EXCEPTION_DATA(unknown_type, std::string);/**< exception members for bad public request type*/
    DECLARE_EXCEPTION_DATA(unknown_registrar, RegistrarId);/**< exception members for bad registrar id*/
    struct Exception
    :   virtual Fred::OperationException,
        ExceptionData_unknown_type< Exception >,
        ExceptionData_unknown_registrar< Exception >
    {};
    CreatePublicRequestAuth(const PublicRequestTypeIface &_type,
                            const std::string &_password);
    CreatePublicRequestAuth(const PublicRequestTypeIface &_type,
                            const std::string &_password,
                            const Optional< std::string > &_reason,
                            const Optional< std::string > &_email_to_answer,
                            const Optional< RegistrarId > &_registrar_id);
    ~CreatePublicRequestAuth() { }
    CreatePublicRequestAuth& set_reason(const std::string &_reason);
    CreatePublicRequestAuth& set_email_to_answer(const std::string &_email);
    CreatePublicRequestAuth& set_registrar_id(RegistrarId _id);
    struct Result
    {
        Result() { }
        Result(const Result &_src);
        Result& operator=(const Result &_src);
        PublicRequestId public_request_id;
        std::string identification;
        std::string password;
    };
    Result exec(OperationContext &_ctx, const PublicRequestObjectLockGuard &_locked_object)const;
private:
    const std::string type_;
    const std::string password_;
    Optional< std::string > reason_;
    Optional< std::string > email_to_answer_;
    Optional< RegistrarId > registrar_id_;
};

}//namespace Fred

#endif//CREATE_PUBLIC_REQUEST_AUTH_H_D034201C95EC2E9FF787F4CBF17E7802
