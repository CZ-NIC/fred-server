#include "src/fredlib/public_request/create_public_request.h"

#include <pthread.h>
#include <cerrno>
#include <cstring>
#include <map>

namespace Fred {

namespace {

enum
{
    SUCCESS =  0,
    FAILURE = -1,
};

class LockGuard
{
public:
    LockGuard(::pthread_mutex_t &_mutex)
    :   mutex_(_mutex)
    {
        const int c_errno = ::pthread_mutex_lock(&mutex_);
        if (c_errno == SUCCESS) {
            return;
        }
        throw std::runtime_error(std::string("pthread_mutex_lock() failure: ") + std::strerror(c_errno));
    }
    ~LockGuard()
    {
        ::pthread_mutex_unlock(&mutex_);
    }
private:
    ::pthread_mutex_t &mutex_;
};

}

std::string prt2str(PublicRequestType _prt)
{
    switch (_prt) {
    case PRT_AUTHINFO_AUTO_RIF:
        return "authinfo_auto_rif";
    case PRT_AUTHINFO_AUTO_PIF:
        return "authinfo_auto_pif";
    case PRT_AUTHINFO_EMAIL_PIF:
        return "authinfo_email_pif";
    case PRT_AUTHINFO_POST_PIF:
        return "authinfo_post_pif";
    case PRT_BLOCK_CHANGES_EMAIL_PIF:
        return "block_changes_email_pif";
    case PRT_BLOCK_CHANGES_POST_PIF:
        return "block_changes_post_pif";
    case PRT_BLOCK_TRANSFER_EMAIL_PIF:
        return "block_transfer_email_pif";
    case PRT_BLOCK_TRANSFER_POST_PIF:
        return "block_transfer_post_pif";
    case PRT_UNBLOCK_CHANGES_EMAIL_PIF:
        return "unblock_changes_email_pif";
    case PRT_UNBLOCK_CHANGES_POST_PIF:
        return "unblock_changes_post_pif";
    case PRT_UNBLOCK_TRANSFER_EMAIL_PIF:
        return "unblock_transfer_email_pif";
    case PRT_UNBLOCK_TRANSFER_POST_PIF:
        return "unblock_transfer_post_pif";
    case PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION:
        return "mojeid_contact_conditional_identification";
    case PRT_MOJEID_CONTACT_IDENTIFICATION:
        return "mojeid_contact_identification";
    case PRT_MOJEID_CONTACT_VALIDATION:
        return "mojeid_contact_validation";
    case PRT_CONTACT_CONDITIONAL_IDENTIFICATION:
        return "contact_conditional_identification";
    case PRT_CONTACT_IDENTIFICATION:
        return "contact_identification";
    case PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER:
        return "mojeid_conditionally_identified_contact_transfer";
    case PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER:
        return "mojeid_identified_contact_transfer";
    case PRT_MOJEID_CONTACT_REIDENTIFICATION:
        return "mojeid_contact_reidentification";
    case PRT_ITEMS:
        break;
    }
    throw BadConversion("prt2str failure: unable convert invalid PublicRequestType to string");
}

PublicRequestType str2prt(const std::string &_str)
{
    typedef std::map< std::string, PublicRequestType > Str2Prt;
    static Str2Prt *convertor_ptr = NULL;
    if (convertor_ptr == NULL) {
        static ::pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        LockGuard lock(mutex);
        if (convertor_ptr == NULL) {
            enum
            {
                PRTID_BEGIN = 0,
                PRTID_END   = PRTID_BEGIN + PRT_ITEMS
            };
            static Str2Prt convertor;
            for (int prt_id = PRTID_BEGIN; prt_id < PRTID_END; ++prt_id) {
                const PublicRequestType prt = static_cast< PublicRequestType >(prt_id);
                const std::string str = prt2str(prt);
                const bool item_already_exists = !convertor.insert(std::make_pair(str, prt)).second;
                if (item_already_exists) {
                    throw std::runtime_error("str2ptr failure: internal error, unable init conversion map");
                }
            }
            convertor_ptr = &convertor;
        }
    }
    Str2Prt::const_iterator prt_ptr = convertor_ptr->find(_str);
    if (prt_ptr != convertor_ptr->end()) {
        return prt_ptr->second;
    }
    throw BadConversion("str2prt failure: unable convert string to PublicRequestType");
}

PublicRequestObjectLockGuard::PublicRequestObjectLockGuard(OperationContext &_ctx, ObjectId _object_id)
:   object_id_(_object_id)
{
    //get lock to the end of transaction for given object
    _ctx.get_conn().exec_params("SELECT lock_public_request_lock($1::BIGINT)",
        Database::query_param_list(object_id_));
}

PublicRequestId create_public_request(
    OperationContext &_ctx,
    const PublicRequestObjectLockGuard &_locked_object,
    const PublicRequestType2 &_type,
    const Optional< std::string > &_reason,
    const Optional< std::string > &_email_to_answer,
    const Optional< RegistrarId > &_registrar_id)
{
    try {
        Database::query_param_list params(_type.get_public_request_type());
        params(_locked_object.get_object_id())
              (_reason.isset() ? _reason.get_value() : Database::QPNull)
              (_email_to_answer.isset() ? _email_to_answer.get_value() : Database::QPNull);
        if (_registrar_id.isset()) {
            params(_registrar_id.get_value());
        }
        else {
            params(Database::QPNull);
        };
        const Database::Result res = _ctx.get_conn().exec_params(
            "WITH request AS ("
                "INSERT INTO public_request "
                    "(request_type,status,resolve_time,reason,email_to_answer,answer_email_id,registrar_id,"
                     "create_request_id,resolve_request_id) "
                "VALUES ((SELECT id FROM enum_public_request_type WHERE name=$1::TEXT),"
                        "(SELECT id FROM enum_public_request_status WHERE name='new'),"
                        "NULL,$3::TEXT,$4::TEXT,NULL,$5::BIGINT,NULL,NULL) "
                "RETURNING id) "
            "INSERT INTO public_request_objects_map (request_id,object_id) "
                "SELECT id,$2::BIGINT FROM request "
            "RETURNING request_id", params);
        const PublicRequestId public_request_id = static_cast< PublicRequestId >(res[0][0]);
        return public_request_id;
    }
    catch (const std::runtime_error &e) {
        throw;
    }
}

}//namespace Fred
