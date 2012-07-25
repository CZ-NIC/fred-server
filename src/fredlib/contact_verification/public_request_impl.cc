#include <stdexcept>

#include "public_request/public_request_impl.h"
#include "types/birthdate.h"
#include "object_states.h"
#include "contact_verification/contact.h"
#include "contact_verification/contact_verification_password.h"
#include "contact_verification/contact_verification.h"
#include "contact_verification/conditional_contact_identification_impl.h"
#include "contact_verification/contact_identification_impl.h"
#include "map_at.h"
#include "factory.h"
#include "contact_verification/public_request_impl.h"

namespace Fred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DEFI(contact_verification)

class ConditionalContactIdentification
        : public Fred::PublicRequest::PublicRequestAuthImpl
        , public Util::FactoryAutoRegister<PublicRequest
              , ConditionalContactIdentification>
{
    Fred::Contact::Verification::ConditionalContactIdentificationImpl cond_contact_identification_impl;
    ContactVerificationPassword contact_verification_passwd_;

public:
    ConditionalContactIdentification()
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
            PRT_CONDITIONAL_CONTACT_IDENTIFICATION,
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

        /* set state */
        insertNewStateRequest(this->getId()
                , this->getObject(0).id
                , ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT);

        /* update states */
        Fred::update_object_states(this->getObject(0).id);

        /* make new request for finishing contact identification */
        PublicRequestAuthPtr new_request(dynamic_cast<PublicRequestAuth*>(
                this->get_manager_ptr()->createRequest(
                        PRT_CONTACT_IDENTIFICATION)));
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
        contact_verification_passwd_.sendEmailPassword("conditional_contact_identification");
        contact_verification_passwd_.sendSmsPassword(
                "Pro aktivaci Vaseho kontaktu je nutne vlozit kody "
                "PIN1 a PIN2. PIN1 Vam byl zaslan emailem, PIN2 je: "
                , "mojeid_pin2");
    }

    static std::string registration_name()
    {
        return Fred::PublicRequest::PRT_CONDITIONAL_CONTACT_IDENTIFICATION;
    }
};



class ContactIdentification
        : public Fred::PublicRequest::PublicRequestAuthImpl
        , public Util::FactoryAutoRegister<PublicRequest, ContactIdentification>
{
    Fred::Contact::Verification::ContactIdentificationImpl contact_identification_impl;
    ContactVerificationPassword contact_verification_passwd_;
public:
    ContactIdentification()
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
                Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION,
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

        /* check if contact is already conditionally identified (21) and cancel state */
        Fred::cancel_object_state(this->getObject(0).id
                , Fred::ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT);

        /* set new state */
        insertNewStateRequest(this->getId()
                , this->getObject(0).id
                , ObjectState::IDENTIFIED_CONTACT);

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
            contact_verification_passwd_.sendEmailPassword("contact_identification");
        }
    }

    static std::string registration_name()
    {
        return Fred::PublicRequest::PRT_CONTACT_IDENTIFICATION;
    }
};

}
}

