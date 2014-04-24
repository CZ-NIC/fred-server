#include "types/birthdate.h"
#include "src/fredlib/object_states.h"
#include "src/fredlib/public_request/public_request_impl.h"
#include "src/fredlib/contact_verification/contact.h"
#include "src/fredlib/contact_verification/contact_verification_password.h"
#include "src/fredlib/contact_verification/contact_conditional_identification_impl.h"
#include "src/fredlib/contact_verification/contact_identification_impl.h"
#include "src/contact_verification/public_request_contact_verification_impl.h"
#include "src/mojeid/request.h"
#include "src/mojeid/mojeid_contact_states.h"
#include "src/mojeid/mojeid_contact_transfer_request_impl.h"
#include "map_at.h"
#include "factory.h"
#include "public_request_verification_impl.h"
#include "mojeid_validators.h"

#include <boost/assign/list_of.hpp>

namespace Fred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DEFI(verification)


class MojeIDConditionallyIdentifiedContactTransfer
        : public Fred::PublicRequest::PublicRequestAuthImpl,
          public Util::FactoryAutoRegister<PublicRequest, MojeIDConditionallyIdentifiedContactTransfer>
{
private:
    Registry::MojeID::MojeIDContactTransferRequestImpl mojeid_transfer_impl_;
    ContactVerificationPassword contact_verification_passwd_;

public:
    MojeIDConditionallyIdentifiedContactTransfer()
        : mojeid_transfer_impl_(this),
          contact_verification_passwd_(this)
    {
    }


    std::string generatePasswords()
    {
        return mojeid_transfer_impl_.generate_passwords();
    }


    void save()
    {
        mojeid_transfer_impl_.pre_save_check();
        if (!this->getId())
        {
            if (!object_has_state(this->getObject(0).id, ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT))
            {
                throw Fred::PublicRequest::NotApplicable("pre_save_check: failed");
            }
            /* if there is another open CICT close it */
            cancel_public_request(this->getObject(0).id,
                    PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER,
                    this->getRequestId());

            /* if there is open CIC request close it */
            cancel_public_request(this->getObject(0).id,
                    PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION,
                    this->getRequestId());
        }
        PublicRequestAuthImpl::save();
    }


    void processAction(bool _check)
    {
        mojeid_transfer_impl_.pre_process_check(_check);
        mojeid_transfer_impl_.process_action(_check);

        /* close CIC request */
        cancel_public_request(this->getObject(0).id,
                PRT_CONTACT_IDENTIFICATION,
                this->getRequestId());

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
    }


    void sendPasswords()
    {
        contact_verification_passwd_.sendEmailPassword("mojeid_verified_contact_transfer");
    }


    static std::string registration_name()
    {
        return PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER;
    }

};



class MojeIDIdentifiedContactTransfer
        : public Fred::PublicRequest::PublicRequestAuthImpl,
          public Util::FactoryAutoRegister<PublicRequest, MojeIDIdentifiedContactTransfer>
{
private:
    Registry::MojeID::MojeIDContactTransferRequestImpl mojeid_transfer_impl_;
    ContactVerificationPassword contact_verification_passwd_;

public:
    MojeIDIdentifiedContactTransfer()
        : mojeid_transfer_impl_(this),
          contact_verification_passwd_(this)
    {
    }


    std::string generatePasswords()
    {
        return mojeid_transfer_impl_.generate_passwords();
    }


    void save()
    {
        mojeid_transfer_impl_.pre_save_check();
        if (!this->getId())
        {
            if (!object_has_state(this->getObject(0).id, ObjectState::IDENTIFIED_CONTACT))
            {
                throw Fred::PublicRequest::NotApplicable("pre_save_check: failed");
            }

            /* if there is another open CICT close it */
            cancel_public_request(this->getObject(0).id,
                    PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER,
                    this->getRequestId());

            /* if there is another open ICT close it */
            cancel_public_request(this->getObject(0).id,
                    PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER,
                    this->getRequestId());

            /* if there is open CIC request close it */
            cancel_public_request(this->getObject(0).id,
                    PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION,
                    this->getRequestId());

        }
        PublicRequestAuthImpl::save();
    }


    void processAction(bool _check)
    {
        mojeid_transfer_impl_.pre_process_check(_check);
        mojeid_transfer_impl_.process_action(_check);
    }


    void sendPasswords()
    {
        contact_verification_passwd_.sendEmailPassword("mojeid_verified_contact_transfer");
    }


    static std::string registration_name()
    {
        return PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER;
    }

};

class MojeIDConditionalContactIdentification
        : public Fred::PublicRequest::PublicRequestAuthImpl
        , public Util::FactoryAutoRegister<PublicRequest
              , MojeIDConditionalContactIdentification>
{
    Fred::Contact::Verification::ConditionalContactIdentificationImpl cond_contact_identification_impl;
    Registry::MojeID::MojeIDContactTransferRequestImpl mojeid_transfer_impl_;
    ContactVerificationPassword contact_verification_passwd_;

public:
    MojeIDConditionalContactIdentification()
    : cond_contact_identification_impl(this
            , Fred::Contact::Verification::create_conditional_identification_validator_mojeid()),
      mojeid_transfer_impl_(this),
      contact_verification_passwd_(this)
    {}

    std::string generatePasswords()
    {
        std::string cci_pass = cond_contact_identification_impl.generate_passwords();
        std::string mtr_pass = mojeid_transfer_impl_.generate_passwords();
        /* merge transfer pin with cond. contact identification */
        return std::string(mtr_pass + cci_pass.substr(mtr_pass.length()));
    }

    void save()
    {
        cond_contact_identification_impl.pre_save_check();
        mojeid_transfer_impl_.pre_save_check();
        if (!this->getId())
        {
            /* if there is another open CIC close it */
            cancel_public_request(this->getObject(0).id,
                    PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION,
                    this->getRequestId());
        }

        PublicRequestAuthImpl::save();
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(boost::format(
                "processing public request id=%1%")
                % this->getId());

        cond_contact_identification_impl.pre_process_check(_check);
        mojeid_transfer_impl_.pre_process_check(_check);
        cond_contact_identification_impl.process_action(_check);
        mojeid_transfer_impl_.process_action(_check);

        cancel_public_request(this->getObject(0).id,
                PRT_CONTACT_CONDITIONAL_IDENTIFICATION,
                this->getRequestId());

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
    }

    void sendPasswords()
    {
        contact_verification_passwd_.sendEmailPassword("mojeid_identification");
        contact_verification_passwd_.sendSmsPassword(
                boost::format(
                "Potvrzujeme uspesne zalozeni uctu mojeID. "
                "Pro aktivaci Vaseho uctu je nutne vlozit kody "
                "PIN1 a PIN2. PIN1 Vam byl zaslan emailem, PIN2 je: %1%"
                )
                , "mojeid_pin2");
    }

    static std::string registration_name()
    {
        return Fred::PublicRequest::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION;
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
    : contact_identification_impl(this
        , Fred::Contact::Verification::create_finish_identification_validator_mojeid())
    , contact_verification_passwd_(this)
    {}

    std::string generatePasswords()
    {
        return contact_identification_impl.generate_passwords();
    }

    void save()
    {
        contact_identification_impl.pre_save_check();
        if (!this->getId())
        {
            if (!object_has_all_of_states(this->getObject(0).id, Util::vector_of<std::string>
                        (ObjectState::SERVER_DELETE_PROHIBITED)
                        (ObjectState::SERVER_UPDATE_PROHIBITED)
                        (ObjectState::SERVER_TRANSFER_PROHIBITED)
                        (::MojeID::ObjectState::MOJEID_CONTACT)))
            {
                throw Fred::PublicRequest::NotApplicable("pre_save_check: failed");
            }

            /* if there is another open CI close it */
            cancel_public_request(this->getObject(0).id, PRT_MOJEID_CONTACT_IDENTIFICATION,
                    this->getRequestId());
        }
        PublicRequestAuthImpl::save();
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(boost::format(
                "processing public request id=%1%")
        % this->getId());

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        contact_identification_impl.pre_process_check(_check);
        contact_identification_impl.process_action(_check);

        /* update states */
        Fred::update_object_states(this->getObject(0).id);
        tx.commit();
    }

    void sendPasswords()
    {
        /* get configured forwarding service type */
        Database::Connection conn = Database::Manager::acquire();
        Database::Result fs = conn.exec(
                "SELECT mtfsm.service_handle"
                " FROM message_type_forwarding_service_map mtfsm"
                " JOIN message_type mt ON mt.id = mtfsm.message_type_id"
                " WHERE mt.type = 'mojeid_pin3'"
                " FOR SHARE OF mtfsm");
        if (fs.size() != 1)
        {
            throw std::runtime_error("message forwarding service not configured for 'mojeid_pin3'!");
        }

        const std::string service = static_cast<std::string>(fs[0]["service_handle"]);

        /* XXX: mapping should be in database */
        typedef std::map<std::string, Fred::Document::GenerationType> ServiceTemplateMap;
        const ServiceTemplateMap service_to_template = boost::assign::map_list_of
            ("POSTSERVIS", Fred::Document::GT_CONTACT_IDENTIFICATION_LETTER_PIN3)
            ("OPTYS", Fred::Document::GT_CONTACT_IDENTIFICATION_LETTER_PIN3_OPTYS);

        ServiceTemplateMap::const_iterator it = service_to_template.find(service);
        if (it == service_to_template.end())
        {
            throw std::runtime_error("unknown mapping for message forwarding service"
                   " to document template for 'mojeid_pin3'!");
        }
        /* contact is already conditionally identified - send pin3 */
        contact_verification_passwd_.sendLetterPassword("pin3", it->second, "mojeid_pin3", "letter");
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

            if (!object_has_state(pri_ptr_->getObject(0).id, ::MojeID::ObjectState::MOJEID_CONTACT))
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

        /* check if contact is already identified (22) and cancel status */
        if (!Fred::object_has_state(pri_ptr_->getObject(0).id
                , Fred::ObjectState::IDENTIFIED_CONTACT))
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
