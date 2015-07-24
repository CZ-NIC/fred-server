#include "src/fredlib/public_request/info_public_request_auth.h"

namespace Fred {

PublicRequestAuthInfo::PublicRequestAuthInfo(OperationContext &_ctx, const PublicRequestLockGuard &_locked)
:   PublicRequestInfo(_ctx, _locked)
{
    try {
        const Database::Result res = _ctx.get_conn().exec_params(
            "SELECT identification,"
                   "password "
            "FROM public_request_auth "
            "WHERE id=$1::BIGINT", Database::query_param_list(id_));
        if (res.size() <= 0) {
            throw std::runtime_error("no public request with authentication found");
        }
        const Database::Row row = res[0];
        identification_ = static_cast< std::string >(row[0]);
        password_       = static_cast< std::string >(row[1]);
    }
    catch (...) {
        throw;
    }
}

PublicRequestAuthInfo::PublicRequestAuthInfo(const PublicRequestAuthInfo &_src)
:   PublicRequestInfo(static_cast< const PublicRequestInfo& >(_src)),
    identification_(_src.identification_),
    password_      (_src.password_)
{ }

PublicRequestAuthInfo& PublicRequestAuthInfo::operator=(const PublicRequestAuthInfo &_src)
{
    static_cast< PublicRequestInfo& >(*this) = static_cast< const PublicRequestInfo& >(_src);
    identification_ = _src.identification_;
    password_       = _src.password_;
    return *this;
}

}//namespace Fred
