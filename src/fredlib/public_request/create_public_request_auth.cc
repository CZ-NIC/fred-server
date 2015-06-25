#include "src/fredlib/public_request/create_public_request_auth.h"
#include "util/random.h"

namespace Fred {

CreatePublicRequestAuth::CreatePublicRequestAuth(const PublicRequestAuthTypeIface &_type)
:   type_(_type.get_public_request_type()),
    password_(_type.generate_passwords())
{
}

CreatePublicRequestAuth::CreatePublicRequestAuth(const PublicRequestAuthTypeIface &_type,
                                                 const Optional< std::string > &_reason,
                                                 const Optional< std::string > &_email_to_answer,
                                                 const Optional< RegistrarId > &_registrar_id)
:   type_(_type.get_public_request_type()),
    password_(_type.generate_passwords()),
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
        Database::query_param_list params(type_);                                           // $1::TEXT
        params(result.identification)                                                       // $2::TEXT
              (password_)                                                                   // $3::TEXT
              (_locked_object.get_object_id())                                              // $4::BIGINT
              (reason_.isset() ? reason_.get_value() : Database::QPNull)                    // $5::TEXT
              (email_to_answer_.isset() ? email_to_answer_.get_value() : Database::QPNull); // $6::TEXT
        if (registrar_id_.isset()) {
            const RegistrarId registrar_id = registrar_id_.get_value();
            const bool registrar_id_exists = static_cast< bool >(_ctx.get_conn().exec_params(
                "SELECT EXISTS(SELECT * FROM registrar WHERE id=$1::BIGINT)",
                Database::query_param_list(registrar_id))[0][0]);
            if (!registrar_id_exists) {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_registrar_id(registrar_id));
            }
            params(registrar_id);                                                           // $7::BIGINT
        }
        else {
            params(Database::QPNull);                                                       // $7::BIGINT
        };
        const Database::Result res = _ctx.get_conn().exec_params(
            "WITH request AS ("
                "INSERT INTO public_request "
                    "(request_type,status,resolve_time,reason,email_to_answer,answer_email_id,registrar_id,"
                     "create_request_id,resolve_request_id) "
                "SELECT eprt.id,eprs.id,NULL,$5::TEXT,$6::TEXT,NULL,$7::BIGINT,NULL,NULL "
                "FROM enum_public_request_type eprt,"
                     "enum_public_request_status eprs "
                "WHERE eprt.name=$1::TEXT AND eprs.name='new' "
                "RETURNING id),"
                 "request_object AS ("
                "INSERT INTO public_request_objects_map (request_id,object_id) "
                    "SELECT id,$4::BIGINT FROM request "
                "RETURNING request_id,object_id) "
            "INSERT INTO public_request_auth (id,identification,password) "
                "SELECT request_id,$2::TEXT,$3::TEXT FROM request_object "
            "RETURNING id,identification,password", params);
        if (0 < res.size()) {
            result.public_request_id = static_cast< PublicRequestId >(res[0][0]);
            result.identification    = static_cast< std::string     >(res[0][1]);
            result.password          = static_cast< std::string     >(res[0][2]);
            return result;
        }
        BOOST_THROW_EXCEPTION(Exception().set_unknown_type(type_));
    }
    catch (const Exception&) {
        throw;
    }
    catch (const std::runtime_error &e) {
        throw;
    }
}

CreatePublicRequestAuth::Result::Result(const Result &_src)
:   public_request_id(_src.public_request_id),
    identification(_src.identification),
    password(_src.password)
{
}

CreatePublicRequestAuth::Result& CreatePublicRequestAuth::Result::operator=(const Result &_src)
{
    public_request_id = _src.public_request_id;
    identification = _src.identification;
    password = _src.password;
    return *this;
}

}//namespace Fred
