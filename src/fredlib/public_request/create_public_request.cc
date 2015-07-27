#include "src/fredlib/public_request/create_public_request.h"
#include "src/fredlib/public_request/public_request_status.h"

namespace Fred {

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
                                          const PublicRequestObjectLockGuard &_locked_object,
                                          const Optional< LogRequestId > &_create_log_request_id)const
{
    try {
        Database::query_param_list params(type_);                                           // $1::TEXT
        params(_locked_object.get_object_id())                                              // $2::BIGINT
              (reason_.isset() ? reason_.get_value() : Database::QPNull)                    // $3::TEXT
              (email_to_answer_.isset() ? email_to_answer_.get_value() : Database::QPNull); // $4::TEXT
        if (registrar_id_.isset()) {
            const RegistrarId registrar_id = registrar_id_.get_value();
            const bool registrar_id_exists = static_cast< bool >(_ctx.get_conn().exec_params(
                "SELECT EXISTS(SELECT * FROM registrar WHERE id=$1::BIGINT)",
                Database::query_param_list(registrar_id))[0][0]);
            if (!registrar_id_exists) {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_registrar_id(registrar_id));
            }
            params(registrar_id);                                                           // $5::BIGINT
        }
        else {
            params(Database::QPNull);                                                       // $5::BIGINT
        }
        if (_create_log_request_id.isset()) {
            params(_create_log_request_id.get_value());                                     // $6::BIGINT
        }
        else {
            params(Database::QPNull);                                                       // $6::BIGINT
        }
        static const std::string prs_new = PublicRequest::Status(PublicRequest::Status::NEW).into< std::string >();
        const Database::Result res = _ctx.get_conn().exec_params(
            "WITH request AS ("
                "INSERT INTO public_request "
                    "(request_type,status,resolve_time,reason,email_to_answer,answer_email_id,registrar_id,"
                     "create_request_id,resolve_request_id) "
                "SELECT eprt.id,eprs.id,NULL,$3::TEXT,$4::TEXT,NULL,$5::BIGINT,$6::BIGINT,NULL "
                "FROM enum_public_request_type eprt,"
                     "enum_public_request_status eprs "
                "WHERE eprt.name=$1::TEXT AND "
                      "eprs.name='" + prs_new + "' "
                "RETURNING id) "
            "INSERT INTO public_request_objects_map (request_id,object_id) "
                "SELECT id,$2::BIGINT FROM request "
            "RETURNING request_id", params);
        if (0 < res.size()) {
            const PublicRequestId public_request_id = static_cast< PublicRequestId >(res[0][0]);
            return public_request_id;
        }
        BOOST_THROW_EXCEPTION(Exception().set_unknown_type(type_));
    }
    catch (const Exception&) {
        throw;
    }
    catch (const std::runtime_error&) {
        throw;
    }
}

}//namespace Fred
