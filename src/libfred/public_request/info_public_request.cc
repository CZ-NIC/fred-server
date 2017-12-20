#include "src/libfred/public_request/info_public_request.hh"

namespace LibFred {

namespace {

template < typename DST >
DST& set(DST &_dst, const Database::Value &_src)
{
    return _dst = static_cast< DST >(_src);
}

template < typename DST, typename INTMDT >
DST& set(DST &_dst, const Database::Value &_src, DST(*convert)(const INTMDT&))
{
    return _dst = convert(static_cast< INTMDT >(_src));
}

typedef bool IsNull;

template < typename DST >
IsNull set(Nullable< DST > &_dst, const Database::Value &_src)
{
    if (_src.isnull()) {
        return true;
    }
    DST dst;
    _dst = set(dst, _src);
    return false;
}

template < typename DST, typename INTMDT >
IsNull set(Nullable< DST > &_dst, const Database::Value &_src, DST(*convert)(const INTMDT&))
{
    if (_src.isnull()) {
        return true;
    }
    DST dst;
    _dst = set(dst, _src, convert);
    return false;
}

} // namespace LibFred::{anonymous}

PublicRequestInfo::PublicRequestInfo(OperationContext &_ctx, const LockedPublicRequest &_locked)
:   id_(_locked.get_id())
{
    try {
        const Database::Result res = _ctx.get_conn().exec_params(
            "SELECT (SELECT name FROM enum_public_request_type WHERE id = request_type) AS request_type,"
                   "create_time,"
                   "(SELECT name FROM enum_public_request_status WHERE id = status) AS status,"
                   "resolve_time,"
                   "reason,"
                   "email_to_answer,"
                   "answer_email_id,"
                   "registrar_id,"
                   "create_request_id,"
                   "resolve_request_id,"
                   "(SELECT object_id FROM public_request_objects_map WHERE request_id=id) AS object_id "
            "FROM public_request "
            "WHERE id=$1::BIGINT", Database::query_param_list(id_));
        if (res.size() <= 0) {
            throw std::runtime_error("no public request found");
        }
        const Database::Row row = res[0];
        set(type_,               row[0]);
        set(create_time_,        row[1], boost::posix_time::time_from_string);
        set(status_,             row[2], Conversion::Enums::from_db_handle< PublicRequest::Status >);
        set(resolve_time_,       row[3], boost::posix_time::time_from_string);
        set(reason_,             row[4]);
        set(email_to_answer_,    row[5]);
        set(answer_email_id_,    row[6]);
        set(registrar_id_,       row[7]);
        set(create_request_id_,  row[8]);
        set(resolve_request_id_, row[9]);
        set(object_id_,          row[10]);
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
    resolve_request_id_(_src.resolve_request_id_),
    object_id_         (_src.object_id_)
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
    object_id_          = _src.object_id_;
    return *this;
}

} // namespace LibFred
