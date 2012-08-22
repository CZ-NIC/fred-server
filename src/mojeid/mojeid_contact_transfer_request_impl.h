#ifndef MOJEID_CONTACT_TRANSFER_REQUEST_H__
#define MOJEID_CONTACT_TRANSFER_REQUEST_H__


#include "public_request/public_request_impl.h"
#include "fredlib/contact_verification/contact_verification_password.h"

namespace Registry {
namespace MojeID {


class MojeIDContactTransferRequestImpl
{
private:
    Fred::PublicRequest::PublicRequestAuthImpl* pra_impl_ptr_;
    Fred::PublicRequest::ContactVerificationPassword contact_verification_passwd_;

public:
    MojeIDContactTransferRequestImpl(
            Fred::PublicRequest::PublicRequestAuthImpl* _pra_impl_ptr);

    void pre_insert_check();

    void pre_save_check();

    void pre_process_check(bool _check);

    void process_action(bool _check);

    std::string generate_passwords();
};


}
}

#endif /*MOJEID_CONTACT_TRANSFER_REQUEST_H__*/

