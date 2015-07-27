#include "src/fredlib/public_request/update_public_request.h"

#include <sstream>

namespace Fred {

UpdatePublicRequest::UpdatePublicRequest(const Optional< PublicRequest::Status::Value > &_status,
                                         const Optional< Nullable< std::string > > &_reason,
                                         const Optional< Nullable< std::string > > &_email_to_answer,
                                         const Optional< Nullable< EmailId > > &_answer_email_id,
                                         const Optional< Nullable< RegistrarId > > &_registrar_id)
:   status_(_status),
    reason_(_reason),
    email_to_answer_(_email_to_answer),
    answer_email_id_(_answer_email_id),
    registrar_id_(_registrar_id)
{
}

UpdatePublicRequest& UpdatePublicRequest::set_status(PublicRequest::Status::Value _status)
{
    status_ = _status;
    return *this;
}

UpdatePublicRequest& UpdatePublicRequest::set_reason(const Nullable< std::string > &_reason)
{
    reason_ = _reason;
    return *this;
}

UpdatePublicRequest& UpdatePublicRequest::set_email_to_answer(const Nullable< std::string > &_email)
{
    email_to_answer_ = _email;
    return *this;
}

UpdatePublicRequest& UpdatePublicRequest::set_answer_email_id(const Nullable< EmailId > &_id)
{
    answer_email_id_ = _id;
    return *this;
}

UpdatePublicRequest& UpdatePublicRequest::set_registrar_id(const Nullable< RegistrarId > _id)
{
    registrar_id_ = _id;
    return *this;
}

UpdatePublicRequest::Result UpdatePublicRequest::exec(OperationContext &_ctx,
                                                      const PublicRequestLockGuard &_locked_public_request,
                                                      const Optional< LogRequestId > &_resolve_log_request_id)const
{
    const PublicRequestId public_request_id = _locked_public_request.get_public_request_id();
    Database::query_param_list params(public_request_id);
    std::ostringstream sql_set;
    Exception bad_params;

    if (status_.isset()) {
        try {
            switch (status_.get_value()) {
            case PublicRequest::Status::ANSWERED:
            case PublicRequest::Status::INVALIDATED:
                break;
            default:
                throw std::runtime_error("unable to set other public request state than 'answered' or 'invalidated'");
            }
            sql_set << "status=(SELECT id FROM enum_public_request_status WHERE name=$"
                    << params.add(PublicRequest::Status(status_.get_value()).into< std::string >())
                    << "::TEXT),"
                       "resolve_time=CASE WHEN status=(SELECT id FROM enum_public_request_status WHERE name=$"
                    << params.add(PublicRequest::Status(PublicRequest::Status::NEW).into< std::string >())
                    << "::TEXT) "
                                         "THEN NOW() "
                                         "ELSE resolve_time "
                                    "END,";
        }
        catch (const std::runtime_error &e) {
            bad_params.set_bad_public_request_status(status_.get_value());
        }
    }


    if (reason_.isset()) {
        sql_set << "reason=$"
                << (reason_.get_value().isnull() ? params.add(Database::QPNull)
                                                 : params.add(reason_.get_value().get_value()))
                << "::TEXT,";
    }

    if (email_to_answer_.isset()) {
        sql_set << "email_to_answer=$"
                << (email_to_answer_.get_value().isnull() ? params.add(Database::QPNull)
                                                          : params.add(email_to_answer_.get_value().get_value()))
                << "::TEXT,";
    }

    if (answer_email_id_.isset()) {
        sql_set << "answer_email_id=$"
                << (answer_email_id_.get_value().isnull() ? params.add(Database::QPNull)
                                                          : params.add(answer_email_id_.get_value().get_value()))
                << "::BIGINT,";
        if (!answer_email_id_.get_value().isnull()) {
            const EmailId email_id = answer_email_id_.get_value().get_value();
            const bool answer_email_id_exists = static_cast< bool >(_ctx.get_conn().exec_params(
                "SELECT EXISTS(SELECT * FROM mail_archive WHERE id=$1::BIGINT)",
                Database::query_param_list(email_id))[0][0]);
            if (!answer_email_id_exists) {
                bad_params.set_unknown_email_id(email_id);
            }
        }
    }

    if (registrar_id_.isset()) {
        sql_set << "registrar_id=$"
                << (registrar_id_.get_value().isnull() ? params.add(Database::QPNull)
                                                       : params.add(registrar_id_.get_value().get_value()))
                << "::BIGINT,";
        if (!registrar_id_.get_value().isnull()) {
            const RegistrarId registrar_id = registrar_id_.get_value().get_value();
            const bool registrar_id_exists = static_cast< bool >(_ctx.get_conn().exec_params(
                "SELECT EXISTS(SELECT * FROM registrar WHERE id=$1::BIGINT)",
                Database::query_param_list(registrar_id))[0][0]);
            if (!registrar_id_exists) {
                bad_params.set_unknown_registrar_id(registrar_id);
            }
        }
    }

    if (_resolve_log_request_id.isset()) {
        sql_set << "resolve_request_id=$" << params.add(_resolve_log_request_id.get_value()) << "::BIGINT,";
    }

    if (sql_set.str().empty()) {
        bad_params.set_nothing_to_do(public_request_id);
    }
    if (bad_params.throw_me()) {
        BOOST_THROW_EXCEPTION(bad_params);
    }
    const std::string to_set = sql_set.str().substr(0, sql_set.str().length() - 1);//last ',' removed
    const Database::Result res = _ctx.get_conn().exec_params(
        "UPDATE public_request pr SET " + to_set + " "
        "WHERE id=$1::BIGINT "
        "RETURNING pr.id,"
                  "(SELECT name FROM enum_public_request_type WHERE id=pr.request_type),"
                  "(SELECT object_id FROM public_request_objects_map WHERE request_id=pr.id)", params);
    if (0 < res.size()) {
        Result result;
        result.public_request_id   = static_cast< PublicRequestId >(res[0][0]);
        result.public_request_type = static_cast< std::string     >(res[0][1]);
        result.object_id           = static_cast< ObjectId        >(res[0][2]);
        return result;
    }
    BOOST_THROW_EXCEPTION(bad_params.set_public_request_doesnt_exist(public_request_id));
}

UpdatePublicRequest::Result::Result(const Result &_src)
:   public_request_id(_src.public_request_id),
    public_request_type(_src.public_request_type),
    object_id(_src.object_id)
{
}

UpdatePublicRequest::Result& UpdatePublicRequest::Result::operator=(const Result &_src)
{
    public_request_id = _src.public_request_id;
    public_request_type = _src.public_request_type;
    object_id = _src.object_id;
    return *this;
}

}//namespace Fred
