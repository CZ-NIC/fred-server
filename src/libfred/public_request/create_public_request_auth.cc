#include "src/libfred/public_request/create_public_request_auth.hh"
#include "src/libfred/public_request/create_public_request.hh"
#include "src/util/random.hh"

namespace LibFred {

CreatePublicRequestAuth::CreatePublicRequestAuth(const Optional< std::string > &_reason,
                                                 const Optional< std::string > &_email_to_answer,
                                                 const Optional< RegistrarId > &_registrar_id)
:   reason_(_reason),
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

CreatePublicRequestAuth& CreatePublicRequestAuth::set_registrar_id(
    OperationContext &_ctx,
    const std::string &_registrar_handle)
{
    const Database::Result res = _ctx.get_conn().exec_params(
        "SELECT id FROM registrar WHERE handle=$1::TEXT", Database::query_param_list(_registrar_handle));
    if (res.size() <= 0) {
        BOOST_THROW_EXCEPTION(Exception().set_unknown_registrar_handle(_registrar_handle));
    }
    registrar_id_ = static_cast< RegistrarId >(res[0][0]);
    return *this;
}

CreatePublicRequestAuth::Result CreatePublicRequestAuth::exec(const LockedPublicRequestsOfObjectForUpdate &_locked_object,
                                                              const PublicRequestAuthTypeIface &_type,
                                                              const Optional< LogRequestId > &_create_log_request_id)const
{
    try {
        CreatePublicRequest::cancel_on_create(_type, _locked_object, registrar_id_, _create_log_request_id);
        Result result;
        result.identification = Random::string_alpha(PUBLIC_REQUEST_AUTH_IDENTIFICATION_LENGTH);
        const std::string public_request_type = _type.get_public_request_type();
        const std::string password = _type.generate_passwords(_locked_object);
        Database::query_param_list params(public_request_type);                             // $1::TEXT
        params(result.identification)                                                       // $2::TEXT
              (password)                                                                    // $3::TEXT
              (_locked_object.get_id())                                                     // $4::BIGINT
              (reason_.isset() ? reason_.get_value() : Database::QPNull)                    // $5::TEXT
              (email_to_answer_.isset() ? email_to_answer_.get_value() : Database::QPNull); // $6::TEXT
        if (registrar_id_.isset()) {
            const RegistrarId registrar_id = registrar_id_.get_value();
            const bool registrar_id_exists = static_cast< bool >(_locked_object.get_ctx().get_conn().exec_params(
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
        if (_create_log_request_id.isset()) {
            params(_create_log_request_id.get_value());                                     // $8::BIGINT
        }
        else {
            params(Database::QPNull);                                                       // $8::BIGINT
        }
        const Database::Result res = _locked_object.get_ctx().get_conn().exec_params(
            "WITH request AS ("
                "INSERT INTO public_request "
                    "(request_type,status,resolve_time,reason,email_to_answer,answer_email_id,registrar_id,"
                     "create_request_id,resolve_request_id) "
                "SELECT eprt.id,eprs.id,NULL,$5::TEXT,$6::TEXT,NULL,$7::BIGINT,$8::BIGINT,NULL "
                "FROM enum_public_request_type eprt,"
                     "enum_public_request_status eprs "
                "WHERE eprt.name=$1::TEXT AND eprs.name='opened' "
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
        BOOST_THROW_EXCEPTION(Exception().set_unknown_type(public_request_type));
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

} // namespace LibFred
