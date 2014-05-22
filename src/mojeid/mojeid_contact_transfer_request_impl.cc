#include "src/fredlib/contact_verification/contact.h"
#include "src/fredlib/object_states.h"
#include "src/mojeid/mojeid_contact_states.h"
#include "src/mojeid/request.h"
#include "src/mojeid/mojeid_contact_transfer_request_impl.h"
#include "src/mojeid/mojeid_validators.h"


namespace Registry {
namespace MojeID {


unsigned long long lock_contact_get_registrar_id(unsigned long long contact_id)
{
    Database::Connection conn = Database::Manager::acquire();

    Database::Result clid_result = conn.exec_params(
            "SELECT o.clid FROM object o JOIN contact c ON c.id = o.id"
            " WHERE c.id = $1::integer FOR UPDATE",
            Database::query_param_list(contact_id));
    if (clid_result.size() != 1) {
        throw std::runtime_error("cannot find contact, object doesn't exist!?"
                " (probably deleted?) - Don't Panic");
    }
    unsigned long long act_registrar = static_cast<unsigned long long>(
        clid_result[0][0]);
    return act_registrar;
}


void run_transfer_command(unsigned long long _registrar_id
    , unsigned long long _old_registrar_id
    , unsigned long long _request_id
    , unsigned long long _contact_id)
{
    /* run transfer command */
    ::MojeID::Request request(205
        , _registrar_id, _request_id);

    Fred::Contact::Verification::contact_transfer(
            request.get_request_id(),
            request.get_registrar_id(),
            _contact_id);

    Fred::Contact::Verification::contact_transfer_poll_message(
            _old_registrar_id, _contact_id);
    request.end_success();
}



MojeIDContactTransferRequestImpl::MojeIDContactTransferRequestImpl(
        Fred::PublicRequest::PublicRequestAuthImpl * _pra_impl_ptr)
    : pra_impl_ptr_(_pra_impl_ptr),
      contact_verification_passwd_(_pra_impl_ptr),
      contact_validator_(Fred::Contact::Verification::create_verified_transfer_validator_mojeid())
{
}


void MojeIDContactTransferRequestImpl::pre_insert_check()
{
    if (Fred::object_has_one_of_states(pra_impl_ptr_->getObject(0).id,
               Util::vector_of<std::string>
                   (Fred::ObjectState::SERVER_TRANSFER_PROHIBITED)
                   (Fred::ObjectState::SERVER_UPDATE_PROHIBITED)
                   (::MojeID::ObjectState::MOJEID_CONTACT)))
    {
        throw Fred::PublicRequest::NotApplicable("pre_save_check: failed");
    }

    Fred::Contact::Verification::Contact cdata = Fred::Contact::Verification::contact_info(
            pra_impl_ptr_->getObject(0).id);
    contact_validator_.check(cdata);
}

void MojeIDContactTransferRequestImpl::pre_save_check()
{
    if (!pra_impl_ptr_->getId())
    {
        pre_insert_check();
    }
}


void MojeIDContactTransferRequestImpl::pre_process_check(bool _check)
{
    pre_insert_check();
}


void MojeIDContactTransferRequestImpl::process_action(bool _check)
{
    unsigned long long act_registrar = lock_contact_get_registrar_id(pra_impl_ptr_->getObject(0).id);
    if (act_registrar != pra_impl_ptr_->getRegistrarId())
    {
        run_transfer_command(
                pra_impl_ptr_->getRegistrarId(),
                act_registrar,
                pra_impl_ptr_->getResolveRequestId(),
                pra_impl_ptr_->getObject(0).id);
    }

    Fred::PublicRequest::insertNewStateRequest(
            pra_impl_ptr_->getId(),
            pra_impl_ptr_->getObject(0).id,
            ::MojeID::ObjectState::MOJEID_CONTACT);

    /* prohibit operations on contact */
    if (Fred::object_has_state(pra_impl_ptr_->getObject(0).id
            , Fred::ObjectState::SERVER_DELETE_PROHIBITED) == false)
    {
        /* set 1 | serverDeleteProhibited */
        Fred::PublicRequest::insertNewStateRequest(pra_impl_ptr_->getId()
                , pra_impl_ptr_->getObject(0).id
                , Fred::ObjectState::SERVER_DELETE_PROHIBITED);
    }
    if (Fred::object_has_state(pra_impl_ptr_->getObject(0).id
            , Fred::ObjectState::SERVER_TRANSFER_PROHIBITED) == false)
    {
        /* set 3 | serverTransferProhibited */
        Fred::PublicRequest::insertNewStateRequest(pra_impl_ptr_->getId()
                , pra_impl_ptr_->getObject(0).id
                , Fred::ObjectState::SERVER_TRANSFER_PROHIBITED);
    }
    if (Fred::object_has_state(pra_impl_ptr_->getObject(0).id
            , Fred::ObjectState::SERVER_UPDATE_PROHIBITED) == false)
    {
        /* set 4 | serverUpdateProhibited */
        Fred::PublicRequest::insertNewStateRequest(pra_impl_ptr_->getId()
                , pra_impl_ptr_->getObject(0).id
                , Fred::ObjectState::SERVER_UPDATE_PROHIBITED);
    }

    /* update states */
    Fred::update_object_states(pra_impl_ptr_->getObject(0).id);

}


std::string MojeIDContactTransferRequestImpl::generate_passwords()
{
    if (pra_impl_ptr_->getPublicRequestManager()->getDemoMode()) {
        return std::string(contact_verification_passwd_.get_password_chunk_length(), '1');
    }
    else {
        return contact_verification_passwd_.generateAuthInfoPassword();
    }
}

}
}

