#ifndef UPDATE_PUBLIC_REQUEST_H_9F964452619CB937F93C2B144C8A204D//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define UPDATE_PUBLIC_REQUEST_H_9F964452619CB937F93C2B144C8A204D

#include "src/fredlib/public_request/public_request_lock_guard.h"
#include "util/optional_value.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace Fred {

enum PublicRequestStatus
{
    PRS_NEW,        ///< Request was created and waiting for autorization 
    PRS_ANSWERED,   ///< Email with answer was sent
    PRS_INVALIDATED ///< Time passed without authorization
};

class PublicRequestStatusBadConversion:public std::runtime_error
{
public:
    PublicRequestStatusBadConversion(const std::string &_message)
    :   std::runtime_error(_message) { }
};

std::string public_request_status2str(PublicRequestStatus _status);
PublicRequestStatus str2public_request_status(const std::string &_status);

class UpdatePublicRequest
{
public:
    typedef boost::posix_time::ptime Time;
    typedef ObjectId EmailId;
    typedef ObjectId RequestId;
    DECLARE_EXCEPTION_DATA(nothing_to_do, PublicRequestId);/**< exception members in case of all items empty*/
    DECLARE_EXCEPTION_DATA(public_request_doesnt_exist, PublicRequestId);/**< exception members in case of bad public_request_id*/
    struct Exception
    :   virtual Fred::OperationException,
        ExceptionData_nothing_to_do< Exception >,
        ExceptionData_public_request_doesnt_exist< Exception >
    {};
    UpdatePublicRequest() { }
    UpdatePublicRequest(const Optional< PublicRequestStatus > &_status,
                        const Optional< Nullable< Time > > &_resolve_time,
                        const Optional< Nullable< std::string > > &_reason,
                        const Optional< Nullable< std::string > > &_email_to_answer,
                        const Optional< Nullable< EmailId > > &_answer_email_id,
                        const Optional< Nullable< RegistrarId > > &_registrar_id,
                        const Optional< Nullable< RequestId > > &_create_request_id,
                        const Optional< Nullable< RequestId > > &_resolve_request_id);
    ~UpdatePublicRequest() { }
    UpdatePublicRequest& set_status(PublicRequestStatus _status);
    UpdatePublicRequest& set_resolve_time(const Nullable< Time > &_time);
    UpdatePublicRequest& set_reason(const Nullable< std::string > &_reason);
    UpdatePublicRequest& set_email_to_answer(const Nullable< std::string > &_email);
    UpdatePublicRequest& set_answer_email_id(const Nullable< EmailId > &_id);
    UpdatePublicRequest& set_registrar_id(const Nullable< RegistrarId > _id);
    UpdatePublicRequest& set_create_request_id(const Nullable< RequestId > &_id);
    UpdatePublicRequest& set_resolve_request_id(const Nullable< RequestId > &_id);
    struct Result
    {
        Result() { }
        Result(const Result &_src);
        Result& operator=(const Result &_src);
        PublicRequestId public_request_id;
        std::string public_request_type;
        ObjectId object_id;
    };
    Result exec(OperationContext &_ctx, const PublicRequestLockGuard &_locked_public_request)const;
private:
    Optional< PublicRequestStatus > status_;
    Optional< Nullable< Time > > resolve_time_;
    Optional< Nullable< std::string > > reason_;
    Optional< Nullable< std::string > > email_to_answer_;
    Optional< Nullable< EmailId > > answer_email_id_;
    Optional< Nullable< RegistrarId > > registrar_id_;
    Optional< Nullable< RequestId > > create_request_id_;
    Optional< Nullable< RequestId > > resolve_request_id_;
};

}//namespace Fred

#endif//UPDATE_PUBLIC_REQUEST_H_9F964452619CB937F93C2B144C8A204D
