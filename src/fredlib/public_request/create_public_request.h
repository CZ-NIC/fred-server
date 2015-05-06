#ifndef CREATE_PUBLIC_REQUEST_H_4C9FE3D9B8BB0233CD814C7F0E46D4C9
#define CREATE_PUBLIC_REQUEST_H_4C9FE3D9B8BB0233CD814C7F0E46D4C9

#include "src/fredlib/opexception.h"
#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"

#include <stdexcept>

namespace Fred {

enum PublicRequestType {
    PRT_AUTHINFO_AUTO_RIF,
    PRT_AUTHINFO_AUTO_PIF,
    PRT_AUTHINFO_EMAIL_PIF,
    PRT_AUTHINFO_POST_PIF,
    PRT_BLOCK_CHANGES_EMAIL_PIF,
    PRT_BLOCK_CHANGES_POST_PIF,
    PRT_BLOCK_TRANSFER_EMAIL_PIF,
    PRT_BLOCK_TRANSFER_POST_PIF,
    PRT_UNBLOCK_CHANGES_EMAIL_PIF,
    PRT_UNBLOCK_CHANGES_POST_PIF,
    PRT_UNBLOCK_TRANSFER_EMAIL_PIF,
    PRT_UNBLOCK_TRANSFER_POST_PIF,
    PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION,
    PRT_MOJEID_CONTACT_IDENTIFICATION,
    PRT_MOJEID_CONTACT_VALIDATION,
    PRT_CONTACT_CONDITIONAL_IDENTIFICATION,
    PRT_CONTACT_IDENTIFICATION,
    PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER,
    PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER,
    PRT_MOJEID_CONTACT_REIDENTIFICATION,
    PRT_ITEMS //doesn't represent any public request type
};

class BadConversion:std::runtime_error
{
public:
    BadConversion(const std::string &_msg):std::runtime_error(_msg) { }
    virtual ~BadConversion()throw() { }
};

std::string prt2str(PublicRequestType _prt);
PublicRequestType str2prt(const std::string &_str);

typedef ::size_t PublicRequestId;
typedef ::size_t RegistrarId;
typedef ::size_t ObjectId;

class PublicRequestLockGuard
{
public:
    PublicRequestLockGuard(OperationContext &_ctx, ObjectId _object_id);
    ObjectId get_object_id()const { return object_id_; }
private:
    const ObjectId object_id_;
};

PublicRequestId create_public_request(
    OperationContext &_ctx,
    const PublicRequestLockGuard &_locked_object,
    PublicRequestType _type,
    const Optional< std::string > &_reason,
    const Optional< std::string > &_email_to_answer,
    const Optional< RegistrarId > &_registrar_id);

}//namespace Fred

#endif//CREATE_PUBLIC_REQUEST_H_4C9FE3D9B8BB0233CD814C7F0E46D4C9
