#include "src/fredlib/public_request/create_public_request.h"
#include "util/random.h"

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
                "SELECT eprt.id,eprs.id,NULL,$3::TEXT,$4::TEXT,NULL,$5::BIGINT,NULL,NULL "
                "FROM enum_public_request_type eprt,"
                     "enum_public_request_status eprs"
                "WHERE eprt.name=$1::TEXT AND eprs.name='new' "
                "RETURNING id) "
            "INSERT INTO public_request_objects_map (request_id,object_id) "
                "SELECT id,$2::BIGINT FROM request "
            "RETURNING request_id", params);
        if (0 < res.size()) {
            const PublicRequestId public_request_id = static_cast< PublicRequestId >(res[0][0]);
            return public_request_id;
        }
        BOOST_THROW_EXCEPTION(Exception().set_bad_type(type_));
    }
    catch (const Exception&) {
        throw;
    }
    catch (const std::runtime_error &e) {
        throw;
    }
}

}//namespace Fred
