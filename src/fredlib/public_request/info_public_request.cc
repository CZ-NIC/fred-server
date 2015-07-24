#include "src/fredlib/public_request/info_public_request.h"

namespace Fred {

PublicRequestInfo::PublicRequestInfo(OperationContext &_ctx, const PublicRequestLockGuard &_locked)
:   id_(_locked.get_public_request_id())
{
    try {
        const Database::Result res = _ctx.get_conn().exec_params(
            "SELECT request_type,"
                   "create_time,"
                   "status,"
                   "resolve_time,"
                   "reason,"
                   "email_to_answer,"
                   "answer_email_id,"
                   "registrar_id,"
                   "create_request_id,"
                   "resolve_request_id "
            "FROM public_request "
            "WHERE id=$1::BIGINT", Database::query_param_list(id_));
        if (res.size() <= 0) {
            throw std::runtime_error("no public request found");
        }
        const Database::Row row = res[0];
        type_ = static_cast< std::string >(row[0]);
        create_time_ = boost::posix_time::time_from_string(static_cast< std::string >(row[1]));
        status_ = PublicRequest::Status::from(static_cast< std::string >(row[2]));
        if (!row[3].isnull()) {
            resolve_time_ = boost::posix_time::time_from_string(static_cast< std::string >(row[3]));
        }
        if (!row[4].isnull()) {
            reason_ = static_cast< std::string >(row[4]);
        }
        if (!row[5].isnull()) {
            email_to_answer_ = static_cast< std::string >(row[5]);
        }
        if (!row[6].isnull()) {
            answer_email_id_ = static_cast< EmailId >(row[6]);
        }
        if (!row[7].isnull()) {
            registrar_id_ = static_cast< RegistrarId >(row[7]);
        }
        if (!row[8].isnull()) {
            create_request_id_ = static_cast< LogRequestId >(row[8]);
        }
        if (!row[9].isnull()) {
            resolve_request_id_ = static_cast< LogRequestId >(row[9]);
        }
    }
    catch (...) {
        throw;
    }
}

PublicRequestInfo::PublicRequestInfo(const PublicRequestInfo &_src)
:   id_                (_src.id_),
    type_              (_src.type_),
    create_time_       (_src.create_time_),
    status_            (_src.status_),
    resolve_time_      (_src.resolve_time_),
    reason_            (_src.reason_),
    email_to_answer_   (_src.email_to_answer_),
    answer_email_id_   (_src.answer_email_id_),
    registrar_id_      (_src.registrar_id_),
    create_request_id_ (_src.create_request_id_),
    resolve_request_id_(_src.resolve_request_id_)
{
}

PublicRequestInfo& PublicRequestInfo::operator=(const PublicRequestInfo &_src)
{
    id_                 = _src.id_;
    type_               = _src.type_;
    create_time_        = _src.create_time_;
    status_             = _src.status_;
    resolve_time_       = _src.resolve_time_;
    reason_             = _src.reason_;
    email_to_answer_    = _src.email_to_answer_;
    answer_email_id_    = _src.answer_email_id_;
    registrar_id_       = _src.registrar_id_;
    create_request_id_  = _src.create_request_id_;
    resolve_request_id_ = _src.resolve_request_id_;
    return *this;
}

}//namespace Fred
