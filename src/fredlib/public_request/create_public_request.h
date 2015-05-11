#ifndef CREATE_PUBLIC_REQUEST_H_4C9FE3D9B8BB0233CD814C7F0E46D4C9//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CREATE_PUBLIC_REQUEST_H_4C9FE3D9B8BB0233CD814C7F0E46D4C9

#include "src/fredlib/opexception.h"
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
private:
    std::string get_public_request_type()const { return "authinfo_auto_rif"; }
};

}//Fred::PublicRequest

typedef ::uint64_t ObjectId;
typedef ObjectId PublicRequestId;
typedef ObjectId RegistrarId;

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

enum { PUBLIC_REQUEST_AUTH_IDENTIFICATION_LENGTH = 32 };

class CreatePublicRequestAuth
{
public:
    DECLARE_EXCEPTION_DATA(bad_type, std::string);/**< exception members for bad public request type*/
    struct Exception
    :   virtual Fred::OperationException,
        ExceptionData_bad_type< Exception >
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
        Result(const Result &_src)
        :   public_request_id(_src.public_request_id),
            identification(_src.identification),
            password(_src.password)
        { }
        Result& operator=(const Result &_src)
        {
            public_request_id = _src.public_request_id;
            identification = _src.identification;
            password = _src.password;
            return *this;
        }
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

#endif//CREATE_PUBLIC_REQUEST_H_4C9FE3D9B8BB0233CD814C7F0E46D4C9
