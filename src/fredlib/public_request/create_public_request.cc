#include "src/fredlib/public_request/create_public_request.h"
#include "util/random.h"

namespace Fred {

PublicRequestObjectLockGuard::PublicRequestObjectLockGuard(OperationContext &_ctx, ObjectId _object_id)
:   object_id_(_object_id)
{
    //get lock to the end of transaction for given object
    _ctx.get_conn().exec_params("SELECT lock_public_request_lock($1::BIGINT)",
        Database::query_param_list(object_id_));
}

CreatePublicRequest::CreatePublicRequest(const PublicRequestTypeIface &_type)
:   type_(_type.get_public_request_type())
{
}

CreatePublicRequest::CreatePublicRequest(const PublicRequestTypeIface &_type,
                                         const Optional< std::string > &_reason,
                                         const Optional< std::string > &_email_to_answer,
                                         const Optional< RegistrarId > &_registrar_id)
:   type_(_type.get_public_request_type()),
    reason_(_reason),
    email_to_answer_(_email_to_answer),
    registrar_id_(_registrar_id)
{
}

CreatePublicRequest& CreatePublicRequest::set_reason(const std::string &_reason)
{
    reason_ = _reason;
    return *this;
}

CreatePublicRequest& CreatePublicRequest::set_email_to_answer(const std::string &_email)
{
    email_to_answer_ = _email;
    return *this;
}

CreatePublicRequest& CreatePublicRequest::set_registrar_id(RegistrarId _id)
{
    registrar_id_ = _id;
    return *this;
}

PublicRequestId CreatePublicRequest::exec(OperationContext &_ctx,
                                          const PublicRequestObjectLockGuard &_locked_object)const
{
    try {
        Database::query_param_list params(type_);
        params(_locked_object.get_object_id())
              (reason_.isset() ? reason_.get_value() : Database::QPNull)
              (email_to_answer_.isset() ? email_to_answer_.get_value() : Database::QPNull);
        if (registrar_id_.isset()) {
            params(registrar_id_.get_value());
        }
        else {
            params(Database::QPNull);
        };
        const Database::Result res = _ctx.get_conn().exec_params(
            "WITH request AS ("
                "INSERT INTO public_request "
                    "(request_type,status,resolve_time,reason,email_to_answer,answer_email_id,registrar_id,"
                     "create_request_id,resolve_request_id) "
                "VALUES ((SELECT id FROM enum_public_request_type WHERE name=$1::TEXT),"
                        "(SELECT id FROM enum_public_request_status WHERE name='new'),"
                        "NULL,$3::TEXT,$4::TEXT,NULL,$5::BIGINT,NULL,NULL) "
                "RETURNING id) "
            "INSERT INTO public_request_objects_map (request_id,object_id) "
                "SELECT id,$2::BIGINT FROM request "
            "RETURNING request_id", params);
        const PublicRequestId public_request_id = static_cast< PublicRequestId >(res[0][0]);
        return public_request_id;
    }
    catch (const std::runtime_error &e) {
        throw;
    }
}

CreatePublicRequestAuth::CreatePublicRequestAuth(const PublicRequestTypeIface &_type,
                                                 const std::string &_password)
:   type_(_type.get_public_request_type()),
    password_(_password)
{
}

CreatePublicRequestAuth::CreatePublicRequestAuth(const PublicRequestTypeIface &_type,
                                                 const std::string &_password,
                                                 const Optional< std::string > &_reason,
                                                 const Optional< std::string > &_email_to_answer,
                                                 const Optional< RegistrarId > &_registrar_id)
:   type_(_type.get_public_request_type()),
    password_(_password),
    reason_(_reason),
    email_to_answer_(_email_to_answer),
    registrar_id_(_registrar_id)
{
}

CreatePublicRequestAuth& CreatePublicRequestAuth::set_reason(const std::string &_reason)
{
    reason_ = _reason;
    return *this;
}

CreatePublicRequestAuth& CreatePublicRequestAuth::set_email_to_answer(const std::string &_email)
{
    email_to_answer_ = _email;
    return *this;
}

CreatePublicRequestAuth& CreatePublicRequestAuth::set_registrar_id(RegistrarId _id)
{
    registrar_id_ = _id;
    return *this;
}

CreatePublicRequestAuth::Result CreatePublicRequestAuth::exec(OperationContext &_ctx,
                                                              const PublicRequestObjectLockGuard &_locked_object)const
{
    try {
        Result result;
        result.identification = Random::string_alpha(PUBLIC_REQUEST_AUTH_IDENTIFICATION_LENGTH);
        Database::query_param_list params(type_);
        params(result.identification)
              (_locked_object.get_object_id())
              (password_)
              (reason_.isset() ? reason_.get_value() : Database::QPNull)
              (email_to_answer_.isset() ? email_to_answer_.get_value() : Database::QPNull);
        if (registrar_id_.isset()) {
            params(registrar_id_.get_value());
        }
        else {
            params(Database::QPNull);
        };
        const Database::Result res = _ctx.get_conn().exec_params(
            "WITH request AS ("
                "INSERT INTO public_request "
                    "(request_type,status,resolve_time,reason,email_to_answer,answer_email_id,registrar_id,"
                     "create_request_id,resolve_request_id) "
                "VALUES ((SELECT id FROM enum_public_request_type WHERE name=$1::TEXT),"
                        "(SELECT id FROM enum_public_request_status WHERE name='new'),"
                        "NULL,$5::TEXT,$6::TEXT,NULL,$7::BIGINT,NULL,NULL) "
                "RETURNING id),"
                 "request_object AS ("
                "INSERT INTO public_request_objects_map (request_id,object_id) "
                    "SELECT id,$3::BIGINT FROM request "
                "RETURNING request_id,object_id) "
            "INSERT INTO public_request_auth (id,identification,password) "
                "SELECT request_id,$2::TEXT,$3::TEXT "
            "RETURNING id,identification,password", params);
        result.public_request_id = static_cast< PublicRequestId >(res[0][0]);
        result.identification    = static_cast< std::string     >(res[0][1]);
        result.password          = static_cast< std::string     >(res[0][2]);
        return result;
    }
    catch (const std::runtime_error &e) {
        throw;
    }
}

}//namespace Fred
