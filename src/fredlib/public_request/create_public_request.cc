#include "src/fredlib/public_request/create_public_request.h"
#include "src/fredlib/public_request/update_public_request.h"
#include "src/fredlib/public_request/public_request_lock_guard.h"
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
        invalidate_the_same(_ctx, type_, _locked_object, registrar_id_, _create_log_request_id);
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
        params(Conversion::Enums::into_string(PublicRequest::Status::NEW));                 // $7::TEXT
        const Database::Result res = _ctx.get_conn().exec_params(
            "WITH request AS ("
                "INSERT INTO public_request "
                    "(request_type,status,resolve_time,reason,email_to_answer,answer_email_id,registrar_id,"
                     "create_request_id,resolve_request_id) "
                "SELECT eprt.id,eprs.id,NULL,$3::TEXT,$4::TEXT,NULL,$5::BIGINT,$6::BIGINT,NULL "
                "FROM enum_public_request_type eprt,"
                     "enum_public_request_status eprs "
                "WHERE eprt.name=$1::TEXT AND "
                      "eprs.name=$7::TEXT "
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

::size_t CreatePublicRequest::invalidate_the_same(OperationContext &_ctx,
                                                  const std::string &_type,
                                                  const PublicRequestObjectLockGuard &_locked_object,
                                                  const Optional< RegistrarId > _registrar_id,
                                                  const Optional< LogRequestId > &_log_request_id)
{
    Database::query_param_list params(_locked_object.get_object_id()); // $1::BIGINT
    params(_type);                                                     // $2::TEXT
    params(Conversion::Enums::into_string(PublicRequest::Status::NEW));// $3::TEXT
    const Database::Result res = _ctx.get_conn().exec_params(
        "SELECT pr.id "
        "FROM public_request pr "
        "JOIN public_request_objects_map prom ON prom.request_id=pr.id "
        "WHERE prom.object_id=$1::BIGINT AND "
              "pr.request_type=(SELECT id FROM enum_public_request_type WHERE name=$2::TEXT) AND "
              "pr.status=(SELECT id FROM enum_public_request_status WHERE name=$3::TEXT)", params);
    for (::size_t idx = 0; idx < res.size(); ++idx) {
        const PublicRequestId public_request_id = static_cast< PublicRequestId >(res[idx][0]);
        UpdatePublicRequest update_public_request_op;
        update_public_request_op.set_status(PublicRequest::Status::INVALIDATED);
        if (_registrar_id.isset()) {
            update_public_request_op.set_registrar_id(_registrar_id.get_value());
        }
        PublicRequestLockGuardById locked_public_request( _ctx, public_request_id);
        update_public_request_op.exec(_ctx, locked_public_request, _log_request_id);
    }
    return res.size();
}

}//namespace Fred
