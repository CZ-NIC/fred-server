#include "public_request/public_request_impl.h"
#include "types/birthdate.h"
#include "object_states.h"
#include "contact_verification/contact.h"
#include "contact_verification/contact_verification_password.h"
#include "contact_verification/contact_verification.h"
#include "contact_verification/conditional_contact_identification_impl.h"
#include "contact_verification/contact_identification_impl.h"
#include "mojeid/request.h"
#include "mojeid/mojeid_contact_states.h"
#include "map_at.h"
#include "factory.h"
#include "public_request_verification_impl.h"

namespace Fred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DEFI(verification)

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



class MojeIDConditionalContactIdentification
        : public Fred::PublicRequest::PublicRequestAuthImpl
        , public Util::FactoryAutoRegister<PublicRequest
              , MojeIDConditionalContactIdentification>
{
    Fred::Contact::Verification::ConditionalContactIdentificationImpl cond_contact_identification_impl;
    ContactVerificationPassword contact_verification_passwd_;

public:
    MojeIDConditionalContactIdentification()
    : cond_contact_identification_impl(this)
    , contact_verification_passwd_(this)
    {}

    std::string generatePasswords()
    {
        return cond_contact_identification_impl.generate_passwords();
    }

    void save()
    {
        cond_contact_identification_impl.pre_save_check();
        /* if there is another open CCI close it */
        cancel_public_request(
            this->getObject(0).id,
            PRT_CONDITIONAL_MOJEID_CONTACT_IDENTIFICATION,
            this->getRequestId());
        PublicRequestAuthImpl::save();
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(boost::format(
                "processing public request id=%1%")
                % this->getId());

        cond_contact_identification_impl.pre_process_check(_check);

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        unsigned long long act_registrar
            = lock_contact_get_registrar_id(this->getObject(0).id);
        if (act_registrar != this->getRegistrarId()) {
            run_transfer_command(this->getRegistrarId()
                , act_registrar,  this->getResolveRequestId()
                , this->getObject(0).id);
        }

        /* set state */
        insertNewStateRequest(this->getId()
                , this->getObject(0).id
                , ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT);

        insertNewStateRequest(this->getId()
                , this->getObject(0).id
                , ::MojeID::MOJEID_CONTACT);

        /* prohibit operations on contact */
        if (object_has_state(this->getObject(0).id
                , ObjectState::SERVER_DELETE_PROHIBITED) == false)
        {
            /* set 1 | serverDeleteProhibited */
            insertNewStateRequest(this->getId()
                    , this->getObject(0).id
                    , ObjectState::SERVER_DELETE_PROHIBITED);
        }
        if (object_has_state(this->getObject(0).id
                , ObjectState::SERVER_TRANSFER_PROHIBITED) == false)
        {
            /* set 3 | serverTransferProhibited */
            insertNewStateRequest(this->getId()
                    , this->getObject(0).id
                    , ObjectState::SERVER_TRANSFER_PROHIBITED);
        }
        if (object_has_state(this->getObject(0).id
                , ObjectState::SERVER_UPDATE_PROHIBITED) == false)
        {
            /* set 4 | serverUpdateProhibited */
            insertNewStateRequest(this->getId()
                    , this->getObject(0).id
                    , ObjectState::SERVER_UPDATE_PROHIBITED);
        }

        /* update states */
        Fred::update_object_states(this->getObject(0).id);

        /* make new request for finishing contact identification */
        PublicRequestAuthPtr new_request(dynamic_cast<PublicRequestAuth*>(
                this->get_manager_ptr()->createRequest(
                        PRT_MOJEID_CONTACT_IDENTIFICATION)));
        if (new_request)
        {
            new_request->setRegistrarId(
                    this->getRegistrarId());
            new_request->setRequestId(
                    this->getResolveRequestId());
            new_request->addObject(
                    this->getObject(0));
            new_request->save();
            new_request->sendPasswords();
        }

        tx.commit();
    }

    void sendPasswords()
    {
        contact_verification_passwd_.sendEmailPassword("mojeid_identification");
        contact_verification_passwd_.sendSmsPassword(
                "Potvrzujeme uspesne zalozeni uctu mojeID. "
                "Pro aktivaci Vaseho uctu je nutne vlozit kody "
                "PIN1 a PIN2. PIN1 Vam byl zaslan emailem, PIN2 je: "
                , "mojeid_pin2");
    }

    static std::string registration_name()
    {
        return Fred::PublicRequest::PRT_CONDITIONAL_MOJEID_CONTACT_IDENTIFICATION;
    }
};

class MojeIDContactIdentification
        : public Fred::PublicRequest::PublicRequestAuthImpl
        , public Util::FactoryAutoRegister<PublicRequest, MojeIDContactIdentification>
{
    Fred::Contact::Verification::ContactIdentificationImpl contact_identification_impl;
    ContactVerificationPassword contact_verification_passwd_;
public:
    MojeIDContactIdentification()
    : contact_identification_impl(this)
    , contact_verification_passwd_(this)
    {}

    std::string generatePasswords()
    {
        return contact_identification_impl.generate_passwords();
    }

    void save()
    {
        contact_identification_impl.pre_save_check();
        /* if there is another open CI close it */
        cancel_public_request(
                this->getObject(0).id,
            PRT_MOJEID_CONTACT_IDENTIFICATION,
            this->getRequestId());
        PublicRequestAuthImpl::save();
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(boost::format(
                "processing public request id=%1%")
        % this->getId());

        contact_identification_impl.pre_process_check(_check);

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        unsigned long long act_registrar
                = lock_contact_get_registrar_id(this->getObject(0).id);
        if (act_registrar != this->getRegistrarId()) {
            run_transfer_command(this->getRegistrarId()
                , act_registrar,  this->getResolveRequestId()
                , this->getObject(0).id);
        }

        /* check if contact is already conditionally identified (21) and cancel state */
        Fred::cancel_object_state(this->getObject(0).id
                , Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT);

        /* set new state */
        insertNewStateRequest(this->getId()
                , this->getObject(0).id
                , ObjectState::IDENTIFIED_CONTACT);

        if (object_has_state(this->getObject(0).id
                , ::MojeID::MOJEID_CONTACT) == false)
        {
            insertNewStateRequest(this->getId()
                    , this->getObject(0).id
                    , ::MojeID::MOJEID_CONTACT);
        }

        /* prohibit operations on contact */
        if (object_has_state(this->getObject(0).id
                , ObjectState::SERVER_DELETE_PROHIBITED) == false)
        {
            /* set 1 | serverDeleteProhibited */
            insertNewStateRequest(this->getId()
                    , this->getObject(0).id
                    , ObjectState::SERVER_DELETE_PROHIBITED);
        }
        if (object_has_state(this->getObject(0).id
                , ObjectState::SERVER_TRANSFER_PROHIBITED) == false)
        {
            /* set 3 | serverTransferProhibited */
            insertNewStateRequest(this->getId()
                    , this->getObject(0).id
                    , ObjectState::SERVER_TRANSFER_PROHIBITED);
        }
        if (object_has_state(this->getObject(0).id
                , ObjectState::SERVER_UPDATE_PROHIBITED) == false)
        {
            /* set 4 | serverUpdateProhibited */
            insertNewStateRequest(this->getId()
                    , this->getObject(0).id
                    , ObjectState::SERVER_UPDATE_PROHIBITED);
        }

        /* update states */
        Fred::update_object_states(this->getObject(0).id);
        tx.commit();
    }

    void sendPasswords()
    {
        /* contact is already conditionally identified - send pin3 */
        contact_verification_passwd_.sendLetterPassword("pin3"
                , Fred::Document::GT_CONTACT_IDENTIFICATION_LETTER_PIN3
                , "mojeid_pin3"
                , "registered_letter"
                );
        /* in demo mode we send pin3 as email attachment */
        if (this->get_manager_ptr()->getDemoMode()) {
            contact_verification_passwd_.sendEmailPassword("mojeid_identification");
        }
    }

    static std::string registration_name()
    {
        return Fred::PublicRequest::PRT_MOJEID_CONTACT_IDENTIFICATION;
    }
};

class MojeIDValidationRequestImpl
{
    PublicRequestImpl* pri_ptr_;
public:
    MojeIDValidationRequestImpl(PublicRequestImpl* _pri_ptr)
    : pri_ptr_(_pri_ptr)
    {}

    bool check() const
    {
        return true;
    }

    void save()
    {
        if (!pri_ptr_->getId()) {

            if(!object_has_one_of_states(
                pri_ptr_->getObject(0).id
                , Util::vector_of<std::string>
                (ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT)// already CI
                (ObjectState::IDENTIFIED_CONTACT))) // already I
            {
                throw NotApplicable("pre_insert_checks: failed!");
            }

            /* has V request */
            if (check_public_request(pri_ptr_->getObject(0).id
                , PRT_MOJEID_CONTACT_VALIDATION) > 0)
            {
                throw RequestExists(PRT_MOJEID_CONTACT_VALIDATION
                        , pri_ptr_->getObject(0).id);
            }
        }
        pri_ptr_->PublicRequestImpl::save();
    }

    virtual void fillTemplateParams(Fred::Mailer::Parameters& params) const
    {
        params["reqdate"] = stringify(pri_ptr_->getCreateTime().date());
        params["reqid"] = stringify(pri_ptr_->getId());
        if (pri_ptr_->getObjectSize()) {
            params["type"] = stringify(pri_ptr_->getObject(0).type);
            params["handle"] = pri_ptr_->getObject(0).handle;
        }
        Database::Connection conn = Database::Manager::acquire();
        Database::Result res = conn.exec_params(
                "SELECT "
                " c.name, c. organization, c.ssn, c.ssntype, "
                " c.street1 || ' ' || COALESCE(c.street2,'') || ' ' ||"
                " COALESCE(c.street3,' ') || ', ' || "
                " c.postalcode || ' ' || c.city || ', ' || c.country "
                "FROM public_request pr"
                " JOIN public_request_objects_map prom ON (prom.request_id=pr.id) "
                " JOIN contact c ON (c.id = prom.object_id) "
                " WHERE pr.id = $1::integer",
                Database::query_param_list(pri_ptr_->getId()));
        if (res.size() == 1) {
            params["name"] = std::string(res[0][0]);
            params["org"] = std::string(res[0][1]);
            params["ic"] = unsigned(res[0][3]) == 4 ? std::string(res[0][2])  : "";
            params["birthdate"] = (unsigned(res[0][3]) == 6
                    ? stringify(birthdate_from_string_to_date(res[0][2]))
                    : std::string(""));
            params["address"] = std::string(res[0][4]);
            params["status"] = pri_ptr_->getStatus() == PRS_ANSWERED ? "1" : "2";
        }
    }

    void invalidateAction()
    {
        LOGGER(PACKAGE).debug(boost::format(
                    "invalidation request id=%1%")
                    % pri_ptr_->getId());
        /* just send email - note that difference between succesfully
         * processed email and invalidated email is done
         * by setting status_ = PRS_INVALID which is passed to email in
         * fillTemplateParams(...) method -
         * (params["status"] = getStatus() == PRS_ANSWERED ? "1" : "2";)
         */
        pri_ptr_->get_answer_email_id() = pri_ptr_->sendEmail();
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(boost::format(
                    "processing validation request id=%1%")
                    % pri_ptr_->getId());

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        if ((object_has_state(pri_ptr_->getObject(0).id
                , ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT) == false)
            && object_has_state(pri_ptr_->getObject(0).id
                , ObjectState::IDENTIFIED_CONTACT) == false)
        {
            throw NotApplicable("cannot process contact validation: no identified state &&"
                    " no conditionally identified state");
        }

        /* check if contact is already conditionally identified (21) and cancel status */
        Fred::cancel_object_state(pri_ptr_->getObject(0).id
                , Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT);

        /* check if contact is already identified (22) and cancel status */
        if (Fred::cancel_object_state(pri_ptr_->getObject(0).id
                , Fred::ObjectState::IDENTIFIED_CONTACT) == false)
        {
            /* otherwise there could be identification request */
            cancel_public_request(pri_ptr_->getObject(0).id
                    , PRT_MOJEID_CONTACT_IDENTIFICATION, pri_ptr_->getResolveRequestId());
        }

        /* set new state */
        insertNewStateRequest(pri_ptr_->getId(), pri_ptr_->getObject(0).id
                , ObjectState::VALIDATED_CONTACT);
        Fred::update_object_states(pri_ptr_->getObject(0).id);
        tx.commit();
    }
};


class MojeIDValidationRequest
    : public PublicRequestImpl,
      public Util::FactoryAutoRegister<PublicRequest, MojeIDValidationRequest>
{
    MojeIDValidationRequestImpl validation_request_impl;
public:

    MojeIDValidationRequest()
    :PublicRequestImpl()
    , validation_request_impl(this)
    {}

    bool check() const
    {
        return validation_request_impl.check();
    }

    void save()
    {
        validation_request_impl.save();
    }

    virtual std::string getTemplateName() const
    {
        return "mojeid_validation";
    }

    virtual void fillTemplateParams(Fred::Mailer::Parameters& params) const
    {
        validation_request_impl.fillTemplateParams(params);
    }

    void invalidateAction()
    {
        validation_request_impl.invalidateAction();
    }

    void processAction(bool _check)
    {
        validation_request_impl.processAction(_check);
    }

    static std::string registration_name()
    {
        return Fred::PublicRequest::PRT_MOJEID_CONTACT_VALIDATION;
    }
};

}
}

