#include "src/backend/contact_verification/public_request_contact_verification_impl.hh"
#include "src/libfred/contact_verification/contact.hh"
#include "src/libfred/contact_verification/contact_conditional_identification_impl.hh"
#include "src/libfred/contact_verification/contact_identification_impl.hh"
#include "src/libfred/contact_verification/contact_verification_password.hh"
#include "src/libfred/contact_verification/contact_verification_validators.hh"
#include "src/libfred/object_states.hh"
#include "src/libfred/public_request/public_request_impl.hh"
#include "src/libfred/registrable_object/contact/undisclose_address.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/factory.hh"
#include "src/util/map_at.hh"
#include "src/util/types/birthdate.hh"

#include <stdexcept>

namespace Fred {
namespace Backend {
namespace ContactVerification {
namespace PublicRequest {

FACTORY_MODULE_INIT_DEFI(contact_verification)

class ConditionalContactIdentification
        : public LibFred::PublicRequest::PublicRequestAuthImpl,
          public ::Util::FactoryAutoRegister<LibFred::PublicRequest::PublicRequest,
                  ConditionalContactIdentification>
{
    LibFred::Contact::Verification::ConditionalContactIdentificationImpl cond_contact_identification_impl;
    LibFred::PublicRequest::ContactVerificationPassword contact_verification_passwd_;

public:
    ConditionalContactIdentification()
        : cond_contact_identification_impl(
                  this,
                  LibFred::Contact::Verification::create_conditional_identification_validator()),
          contact_verification_passwd_(this)
    {
    }

    std::string generatePasswords()
    {
        return cond_contact_identification_impl.generate_passwords();
    }

    void save()
    {
        cond_contact_identification_impl.pre_save_check();
        if (!this->getId())
        {
            /* if there is another open CCI close it */
            LibFred::PublicRequest::cancel_public_request(
                    this->getObject(0).id,
                    PRT_CONTACT_CONDITIONAL_IDENTIFICATION,
                    this->getRequestId());
        }
        PublicRequestAuthImpl::save();
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(
                boost::format("processing public request id=%1%")
                % this->getId());

        cond_contact_identification_impl.pre_process_check(_check);
        cond_contact_identification_impl.process_action(_check);

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        /* update states */
        LibFred::update_object_states(this->getObject(0).id);

        /* make new request for finishing contact identification */
        LibFred::PublicRequest::PublicRequestAuthPtr new_request(dynamic_cast<PublicRequestAuth*>(
                    this->get_manager_ptr()->createRequest(PRT_CONTACT_IDENTIFICATION)));
        if (new_request)
        {
            new_request->setRegistrarId(this->getRegistrarId());
            new_request->setRequestId(this->getResolveRequestId());
            new_request->addObject(this->getObject(0));
            new_request->save();
            new_request->sendPasswords();
        }

        tx.commit();
    }

    void sendPasswords()
    {
        contact_verification_passwd_.sendEmailPassword("conditional_contact_identification");
        contact_verification_passwd_.sendSmsPassword(
                boost::format(
                        "Potvrzujeme zahajeni procesu verifikace kontaktu v registru domen. "
                        "Prvni krok spociva v zadani PIN1 a PIN2. PIN1 Vam byl zaslan e-mailem, PIN2 je: %1%"),
                "contact_verification_pin2");
    }

    static std::string registration_name()
    {
        return Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_CONDITIONAL_IDENTIFICATION;
    }

};


class ContactIdentification
        : public LibFred::PublicRequest::PublicRequestAuthImpl,
          public ::Util::FactoryAutoRegister<LibFred::PublicRequest::PublicRequest,
                  ContactIdentification>
{
    LibFred::Contact::Verification::ContactIdentificationImpl contact_identification_impl;
    LibFred::PublicRequest::ContactVerificationPassword contact_verification_passwd_;

public:
    ContactIdentification()
        : contact_identification_impl(
                  this,
                  LibFred::Contact::Verification::create_finish_identification_validator()),
          contact_verification_passwd_(this)
    {
    }

    std::string generatePasswords()
    {
        return contact_identification_impl.generate_passwords();
    }

    void save()
    {
        contact_identification_impl.pre_save_check();
        if (!this->getId())
        {
            /* if there is another open CI close it */
            LibFred::PublicRequest::cancel_public_request(
                    this->getObject(0).id,
                    Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_IDENTIFICATION,
                    this->getRequestId());
        }
        PublicRequestAuthImpl::save();
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(
                boost::format("processing public request id=%1%")
                % this->getId());

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        contact_identification_impl.pre_process_check(_check);
        contact_identification_impl.process_action(_check);

        /* update states */
        LibFred::update_object_states(this->getObject(0).id);

        tx.commit();
    }

    void sendPasswords()
    {
        /* contact is already conditionally identified - send pin3 */
        contact_verification_passwd_.sendLetterPassword(
                "pin3",
                LibFred::Document::GT_CONTACT_VERIFICATION_LETTER_PIN3,
                "contact_verification_pin3",
                "letter");
        // send email with url in contact identification
        contact_verification_passwd_.sendEmailPassword("contact_identification");
    }

    static std::string registration_name()
    {
        return Fred::Backend::ContactVerification::PublicRequest::PRT_CONTACT_IDENTIFICATION;
    }

};

} // namespace Fred::Backend::ContactVerification::PublicRequest
} // namespace Fred::Backend::ContactVerification
} // namespace Fred::Backend
} // namespace Fred
