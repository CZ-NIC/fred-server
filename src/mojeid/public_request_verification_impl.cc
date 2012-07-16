#include "public_request/public_request_impl.h"
#include "types/birthdate.h"
#include "object_states.h"
#include "contact_verification/contact.h"
#include "contact_verification/contact_verification.h"
#include "mojeid/request.h"
#include "mojeid/mojeid_contact_states.h"
#include "map_at.h"
#include "factory.h"
#include "public_request_verification_impl.h"

namespace Fred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DEFI(verification)

class ContactVerificationPassword
{
    PublicRequestAuthImpl* prai_ptr_;

public:

    typedef std::map<std::string, std::string> MessageData;

private:

    const MessageData collectMessageData()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result result = conn.exec_params(
                "SELECT c.name, c.organization, c.street1, c.city,"
                " c.stateorprovince, c.postalcode, c.country, c.email,"
                " oreg.historyid, c.telephone, ec.country, ec.country_cs"
                " FROM contact c"
                " JOIN object_registry oreg ON oreg.id = c.id"
                " JOIN enum_country ec ON ec.id = c.country "
                " WHERE c.id = $1::integer",
                Database::query_param_list(prai_ptr_->getObject(0).id));
        if (result.size() != 1)
            throw std::runtime_error("unable to get data for"
                    " password messages");

        MessageData data;

        std::string name = static_cast<std::string>(result[0][0]);
        std::size_t pos = name.find_last_of(" ");
        data["firstname"] = name.substr(0, pos);
        data["lastname"] = name.substr(pos + 1);
        data["organization"] = static_cast<std::string>(result[0][1]);
        data["street"] = static_cast<std::string>(result[0][2]);
        data["city"] = static_cast<std::string>(result[0][3]);
        data["stateorprovince"] = static_cast<std::string>(result[0][4]);
        data["postalcode"] = static_cast<std::string>(result[0][5]);
        data["country"] = static_cast<std::string>(result[0][6]);
        data["email"] = static_cast<std::string>(result[0][7]);
        data["hostname"] = prai_ptr_->getPublicRequestManager()\
                ->getIdentificationMailAuthHostname();
        data["identification"] = prai_ptr_->getIdentification();
        data["handle"] = boost::algorithm::to_lower_copy(
                prai_ptr_->getObject(0).handle);
        /* password split */
        const std::string password = prai_ptr_->getPassword();
        data["pin1"] = password.substr(
                0, -get_password_chunk_length() + password.length());
        data["pin2"] = password.substr(
                -get_password_chunk_length() + password.length());
        data["pin3"] = password;
        data["reqdate"] = boost::gregorian::to_iso_extended_string(
                prai_ptr_->getCreateTime().date());
        data["contact_id"] = boost::lexical_cast<std::string>(
                prai_ptr_->getObject(0).id);
        data["contact_hid"] = static_cast<std::string>(result[0][8]);
        data["phone"] = static_cast<std::string>(result[0][9]);
        data["country_name"] = static_cast<std::string>(result[0][10]);
        data["country_cs_name"] = static_cast<std::string>(result[0][11]);

        return data;
    }

public:

    size_t get_password_chunk_length()
    {
        static const size_t PASSWORD_CHUNK_LENGTH = 8;
        return PASSWORD_CHUNK_LENGTH;
    }

    ContactVerificationPassword(PublicRequestAuthImpl* _prai_ptr)
    : prai_ptr_(_prai_ptr)
    {}

    void sendEmailPassword(const std::string& mailTemplate //db table mail_type.name
            )
    {
        LOGGER(PACKAGE).debug("public request auth - send email password");

        MessageData data = this->collectMessageData();

        Fred::Mailer::Attachments attach;
        Fred::Mailer::Handles handles;
        Fred::Mailer::Parameters params;

        params["firstname"] = map_at(data, "firstname");
        params["lastname"]  = map_at(data, "lastname");
        params["email"]     = map_at(data, "email");
        params["hostname"]  = map_at(data, "hostname");
        params["handle"]    = map_at(data, "handle");
        params["identification"] = map_at(data, "identification");
        params["passwd"]    = map_at(data, "pin1");

        Database::Connection conn = Database::Manager::acquire();

        /* for demo purpose we send second half of password as well */
        if (prai_ptr_->getPublicRequestManager()->getDemoMode() == true)
        {
            if(map_at(data, "pin1").empty())
            {
                params["passwd3"] = map_at(data, "pin3");
            }
            else
            {
                params["passwd2"] = map_at(data, "pin2");
            }
            unsigned long long file_id = 0;

            Database::Result result = conn.exec_params(
                    "SELECT la.file_id FROM letter_archive la "
                    " JOIN message_archive ma ON ma.id=la.id "
                    " JOIN public_request_messages_map prmm "
                    " ON prmm.message_archive_id = ma.id "
                    " WHERE prmm.public_request_id = $1::integer "
                    " AND prmm.message_archive_id is not null",
                    Database::query_param_list(prai_ptr_->getId()));
            if (result.size() == 1)
            {
                file_id = result[0][0];
                attach.push_back(file_id);
            }
        }

        handles.push_back(prai_ptr_->getObject(0).handle);

        unsigned long long id = prai_ptr_->getPublicRequestManager()
            ->getMailerManager()->sendEmail(
                "",           /* default sender */
                params["email"],
                "",           /* default subject */
                mailTemplate,
                params,
                handles,
                attach
                );

        Database::Transaction tx(conn);
        conn.exec_params("INSERT INTO public_request_messages_map "
                " (public_request_id, message_archive_id, mail_archive_id) "
                " VALUES ($1::integer, $2::integer, $3::integer)",
                Database::query_param_list
                    (prai_ptr_->getId())
                    (Database::QPNull)
                    (id));
        tx.commit();
    }

    void sendLetterPassword( const std::string& custom_tag //tag in template xml params: "pin2",  "pin3"
            , Fred::Document::GenerationType doc_type //type for document generator
            , const std::string& message_type //for message_archive: "mojeid_pin2", "mojeid_pin3"
            , const std::string& comm_type //for message_archive: pin2 "letter" or pin3 "registered_letter"
            )
    {
        LOGGER(PACKAGE).debug("public request auth - send letter password");

        MessageData data = collectMessageData();

        std::stringstream xmldata;

        std::string addr_country = ((map_at(data, "country_cs_name")).empty()
                ? map_at(data, "country_name")
                : map_at(data, "country_cs_name"));

        xmldata << "<?xml version='1.0' encoding='utf-8'?>"
                 << "<mojeid_auth>"
                 << "<user>"
                 << "<actual_date>" << map_at(data, "reqdate")
                     << "</actual_date>"
                 << "<name>" << map_at(data, "firstname")
                             << " " << map_at(data, "lastname") << "</name>"
                 << "<organization>" << map_at(data, "organization")
                     << "</organization>"
                 << "<street>" << map_at(data, "street") << "</street>"
                 << "<city>" << map_at(data, "city") << "</city>"
                 << "<stateorprovince>" << map_at(data, "stateorprovince")
                     << "</stateorprovince>"
                 << "<postal_code>" << map_at(data, "postalcode")
                     << "</postal_code>"
                 << "<country>" << addr_country << "</country>"
                 << "<account>"
                 << "<username>" << map_at(data, "handle") << "</username>"
                 << "<first_name>" << map_at(data, "firstname")
                     << "</first_name>"
                 << "<last_name>" << map_at(data, "lastname") << "</last_name>"
                 << "<email>" << map_at(data, "email") << "</email>"
                 << "</account>"
                 << "<auth>"
                 << "<codes>"
                 << std::string("<") + custom_tag + ">" + map_at(data, custom_tag) + ("</") + custom_tag + ">"
                 << "</codes>"
                 << "<link>" << map_at(data, "hostname") << "</link>"
                 << "</auth>"
                 << "</user>"
                 << "</mojeid_auth>";

            unsigned long long file_id = prai_ptr_->getPublicRequestManager()
                ->getDocumentManager()->generateDocumentAndSave(
                    doc_type,
                    xmldata,
                    "identification_request-"
                        + boost::lexical_cast<std::string>(prai_ptr_->getId())
                        + ".pdf",
                    7,
                    "");

            Fred::Messages::PostalAddress pa;
            pa.name    = map_at(data, "firstname") + " "
                    + map_at(data, "lastname");
            pa.org     = map_at(data, "organization");
            pa.street1 = map_at(data, "street");
            pa.street2 = std::string("");
            pa.street3 = std::string("");
            pa.city    = map_at(data, "city");
            pa.state   = map_at(data, "stateorprovince");
            pa.code    = map_at(data, "postalcode");
            pa.country = map_at(data, "country_name");

            unsigned long long message_id =
                prai_ptr_->getPublicRequestManager()->getMessagesManager()
                    ->save_letter_to_send(
                        map_at(data, "handle").c_str()//contact handle
                        , pa
                        , file_id
                        , message_type.c_str()
                        , boost::lexical_cast<unsigned long >(map_at(data
                                , "contact_id"))//contact object_registry.id
                        , boost::lexical_cast<unsigned long >(map_at(data
                                , "contact_hid"))//contact_history.historyid
                        , comm_type.c_str()//comm_type letter or registered_letter
                        );

            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);
            conn.exec_params("INSERT INTO public_request_messages_map "
                    " (public_request_id, message_archive_id, mail_archive_id) "
                    " VALUES ($1::integer, $2::integer, $3::integer)",
                    Database::query_param_list
                        (prai_ptr_->getId())
                        (message_id)
                        (Database::QPNull));
            tx.commit();
    }


    void sendSmsPassword(const std::string& sms_template
            , const std::string& message_type //for message_archive: "mojeid_pin2"
            )
    {
        LOGGER(PACKAGE).debug("public request auth - send sms password");

        MessageData data = collectMessageData();

        unsigned long long message_id =
                prai_ptr_->getPublicRequestManager()->getMessagesManager()
                    ->save_sms_to_send(
                map_at(data, "handle").c_str()
                , map_at(data, "phone").c_str()
                , (sms_template + map_at(data, "pin2")).c_str()
                , message_type.c_str()//"mojeid_pin2"
                , boost::lexical_cast<unsigned long >(map_at(data
                        , "contact_id"))
                , boost::lexical_cast<unsigned long >(map_at(data
                        , "contact_hid"))
                );

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        conn.exec_params("INSERT INTO public_request_messages_map "
                " (public_request_id, message_archive_id, mail_archive_id) "
                " VALUES ($1::integer, $2::integer, $3::integer)",
                Database::query_param_list
                    (prai_ptr_->getId())
                    (message_id)
                    (Database::QPNull));
        tx.commit();
    }


    std::string generateRandomPassword(const size_t _length)
    {
        return Random::string_from(_length,
                "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789");
    }


    std::string generateRandomPassword()
    {
        return generateRandomPassword(get_password_chunk_length());
    }


    std::string generateAuthInfoPassword()
    {
        unsigned long long contact_id = prai_ptr_->getObject(0).id;

        Database::Connection conn = Database::Manager::acquire();
        Database::Result rauthinfo = conn.exec_params(
                "SELECT substr(replace(o.authinfopw, ' ', ''), 1, $1::integer) "
                " FROM object o JOIN contact c ON c.id = o.id"
                " WHERE c.id = $2::integer",
                Database::query_param_list(get_password_chunk_length())
                                          (contact_id));
        if (rauthinfo.size() != 1)
        {
            throw std::runtime_error(str(boost::format(
                        "cannot retrieve authinfo for contact id=%1%")
                        % contact_id));
        }
        std::string passwd;
        /* pin1 */
        if (rauthinfo[0][0].isnull())\
        {
            passwd = generateRandomPassword(get_password_chunk_length());
        }
        else
        {
            passwd = static_cast<std::string>(rauthinfo[0][0]);
            LOGGER(PACKAGE).debug(boost::format("authinfo w/o spaces='%s'")
                % passwd);
            /* fill with random to PASSWORD_CHUNK_LENGTH size */
            size_t to_fill = 0;
            if ((to_fill = (get_password_chunk_length() - passwd.length())
                    ) > 0)
            {
                passwd += generateRandomPassword(to_fill);
                LOGGER(PACKAGE).debug(boost::format("authinfo filled='%s'")
                    % passwd);
            }
        }
        /* append pin2 */
        passwd += generateRandomPassword(get_password_chunk_length());
        return passwd;
    }
};//class ContactVerificationPassword



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


class ConditionalContactIdentificationImpl
{
    Fred::PublicRequest::PublicRequestAuthImpl* pra_impl_ptr_;
    ContactVerificationPassword contact_verification_passwd_;
    Fred::Contact::Verification::ContactValidator contact_validator_;
public:
    ConditionalContactIdentificationImpl(
            Fred::PublicRequest::PublicRequestAuthImpl* _pra_impl_ptr)
    : pra_impl_ptr_(_pra_impl_ptr)
    , contact_verification_passwd_(_pra_impl_ptr)

    , contact_validator_(Fred::Contact::Verification
            ::create_conditional_identification_validator())
    {}

    std::string generatePasswords()
    {
        if(pra_impl_ptr_->getPublicRequestManager()->getDemoMode())
        {
            return std::string(contact_verification_passwd_
                    .get_password_chunk_length(),'1')//pin1:11111111
                +std::string(contact_verification_passwd_
                    .get_password_chunk_length(),'2'); //pin2:22222222
        }
        else
        {
            return contact_verification_passwd_.generateAuthInfoPassword();
        }
    }

    void pre_save_check()
    {
        /* insert */
        if (!pra_impl_ptr_->getId())
        {
            Fred::Contact::Verification::Contact cdata
                = Fred::Contact::Verification::contact_info(
                        pra_impl_ptr_->getObject(0).id);
            contact_validator_.check(cdata);

            if((object_has_one_of_states(pra_impl_ptr_->getObject(0).id
                , Util::vector_of<std::string>
                (ObjectState::SERVER_TRANSFER_PROHIBITED)//3 | serverTransferProhibited
                (ObjectState::SERVER_UPDATE_PROHIBITED)//4 | serverUpdateProhibited
            )
            ||
            (object_has_one_of_states(pra_impl_ptr_->getObject(0).id
                , Util::vector_of<std::string>
                (ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT)// already CI
                (ObjectState::IDENTIFIED_CONTACT)// already I
                (ObjectState::VALIDATED_CONTACT))// already V
            )))
            {
                throw NotApplicable("pre_save_check: failed!");
            }
        }
    }

    void pre_process_check(bool _check)
    {
        /* object should not change */
        if (object_was_changed_since_request_create(
                pra_impl_ptr_->getId())) {
            throw ObjectChanged();
        }

        Fred::Contact::Verification::Contact cdata
            = Fred::Contact::Verification::contact_info(
                    pra_impl_ptr_->getObject(0).id);
        contact_validator_.check(cdata);

    }
};

class ConditionalContactIdentification
        : public Fred::PublicRequest::PublicRequestAuthImpl
        , public Util::FactoryAutoRegister<PublicRequest
              , ConditionalContactIdentification>
{
    ConditionalContactIdentificationImpl cond_contact_identification_impl;
    ContactVerificationPassword contact_verification_passwd_;

public:
    ConditionalContactIdentification()
    : cond_contact_identification_impl(this)
    , contact_verification_passwd_(this)
    {}

    std::string generatePasswords()
    {
        return cond_contact_identification_impl.generatePasswords();
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
        contact_verification_passwd_.sendEmailPassword("mojeid_identification");
        contact_verification_passwd_.sendSmsPassword(
                "Potvrzujeme uspesne zalozeni uctu mojeID. "
                "Pro aktivaci Vaseho uctu je nutne vlozit kody "
                "PIN1 a PIN2. PIN1 Vam byl zaslan emailem, PIN2 je: "
                , "mojeid_pin2");
    }

    static std::string registration_name()
    {
        return PRT_CONDITIONAL_CONTACT_IDENTIFICATION;
    }
};


class ContactIdentificationImpl
{
    Fred::PublicRequest::PublicRequestAuthImpl* pra_impl_ptr_;
    ContactVerificationPassword contact_verification_passwd_;
    Fred::Contact::Verification::ContactValidator contact_validator_;
public:
    ContactIdentificationImpl(
        Fred::PublicRequest::PublicRequestAuthImpl* _pra_impl_ptr)
    : pra_impl_ptr_(_pra_impl_ptr)
    , contact_verification_passwd_(_pra_impl_ptr)
    , contact_validator_(Fred::Contact::Verification::create_finish_identification_validator())
    {}

    std::string generatePasswords()
    {
        /* generate pin3 */
        if(pra_impl_ptr_->getPublicRequestManager()
                ->getDemoMode())
        {
            return std::string(contact_verification_passwd_
                    .get_password_chunk_length(),'3');//pin3:33333333
        }
        else
        {
            return contact_verification_passwd_.generateRandomPassword();
        }
    }

    void pre_save_check()
    {
        if (!pra_impl_ptr_->getId())
        {
            Fred::Contact::Verification::Contact cdata
                = Fred::Contact::Verification::contact_info(
                        pra_impl_ptr_->getObject(0).id);
            contact_validator_.check(cdata);

            /* don't check this when contact is already CI - we are creating
             * I request only for finishing identification - pin3 */
            if (((object_has_state(pra_impl_ptr_->getObject(0).id
                    , ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT) == false)
                ||
                (object_has_one_of_states(
                    pra_impl_ptr_->getObject(0).id
                    , Util::vector_of<std::string>
                    (ObjectState::SERVER_TRANSFER_PROHIBITED)
                    (ObjectState::SERVER_UPDATE_PROHIBITED)) == false)
                ||
                object_has_one_of_states(
                    pra_impl_ptr_->getObject(0).id
                    , Util::vector_of<std::string>
                    (ObjectState::IDENTIFIED_CONTACT) // already I
                    (ObjectState::VALIDATED_CONTACT))// already V
            ))
            {
                throw NotApplicable("pre_save_check: failed!");
            }
        }
    }

    void pre_process_check(bool _check)
    {
        /* object should not change */
        if (object_has_state(pra_impl_ptr_->getObject(0).id
                , ObjectState::CONDITIONALLY_IDENTIFIED_CONTACT) == false
                && object_was_changed_since_request_create(pra_impl_ptr_->getId()))
        {
            throw ObjectChanged();
        }

        Fred::Contact::Verification::Contact cdata
            = Fred::Contact::Verification::contact_info(
                    pra_impl_ptr_->getObject(0).id);
        contact_validator_.check(cdata);


    }
};

class ContactIdentification
        : public Fred::PublicRequest::PublicRequestAuthImpl
        , public Util::FactoryAutoRegister<PublicRequest, ContactIdentification>
{
    ContactIdentificationImpl contact_identification_impl;
    ContactVerificationPassword contact_verification_passwd_;
public:
    ContactIdentification()
    : contact_identification_impl(this)
    , contact_verification_passwd_(this)
    {}

    std::string generatePasswords()
    {
        return contact_identification_impl.generatePasswords();
    }

    void save()
    {
        contact_identification_impl.pre_save_check();
        /* if there is another open CI close it */
        cancel_public_request(
                this->getObject(0).id,
            PRT_CONTACT_IDENTIFICATION,
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
        return PRT_CONTACT_IDENTIFICATION;
    }
};

class ValidationRequestImpl
{
    PublicRequestImpl* pri_ptr_;
public:
    ValidationRequestImpl(PublicRequestImpl* _pri_ptr)
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
                , PRT_CONTACT_VALIDATION) > 0)
            {
                throw RequestExists(PRT_CONTACT_VALIDATION
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
                    , PRT_CONTACT_IDENTIFICATION, pri_ptr_->getResolveRequestId());
        }

        /* set new state */
        insertNewStateRequest(pri_ptr_->getId(), pri_ptr_->getObject(0).id
                , ObjectState::VALIDATED_CONTACT);
        Fred::update_object_states(pri_ptr_->getObject(0).id);
        tx.commit();
    }
};


class ValidationRequest
    : public PublicRequestImpl,
      public Util::FactoryAutoRegister<PublicRequest, ValidationRequest>
{
    ValidationRequestImpl validation_request_impl;
public:

    ValidationRequest()
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
        return PRT_CONTACT_VALIDATION;
    }
};

}
}

