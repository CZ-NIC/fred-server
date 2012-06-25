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

struct LetterType
{
    enum Type
    {
        LETTER_PIN2,
        LETTER_PIN3
    };
};

struct EmailType
{
    enum Type
    {
        EMAIL_PIN2_SMS,
        EMAIL_PIN2_LETTER
    };

};

class ContactVerificationPimpl
{
    PublicRequestAuthImpl* prai_ptr_;

    Fred::Contact::Verification::ContactValidator contact_validator_;

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
        data["hostname"] = prai_ptr_->getPublicRequestManager()->getIdentificationMailAuthHostname();
        data["identification"] = prai_ptr_->getIdentification();
        data["handle"] = boost::algorithm::to_lower_copy(prai_ptr_->getObject(0).handle);
        /* password split */
        const std::string password = prai_ptr_->getPassword();
        data["pin1"] = password.substr(0, -get_password_chunk_length() + password.length());
        data["pin2"] = password.substr(-get_password_chunk_length() + password.length());
        data["pin3"] = password;
        data["reqdate"] = boost::gregorian::to_iso_extended_string(prai_ptr_->getCreateTime().date());
        data["contact_id"] = boost::lexical_cast<std::string>(prai_ptr_->getObject(0).id);
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

    Fred::Contact::Verification::ContactValidator& get_contact_validator()
    {
        return contact_validator_;
    }

    ContactVerificationPimpl(PublicRequestAuthImpl* _prai_ptr)
    : prai_ptr_(_prai_ptr)
    {}


    void sendEmailPassword(const EmailType::Type &_type)
    {
        LOGGER(PACKAGE).debug("public request auth - send email password");

        MessageData data = this->collectMessageData();

        Fred::Mailer::Attachments attach;
        Fred::Mailer::Handles handles;
        Fred::Mailer::Parameters params;

        unsigned short type = ((_type == EmailType::EMAIL_PIN2_SMS) ? 1
                                : (_type == EmailType::EMAIL_PIN2_LETTER) ? 2 : 0);
        if (type == 0) {
            throw std::runtime_error("unknown mail type (pin2 - sms/letter)");
        }
        params["rtype"]     = boost::lexical_cast<std::string>(type);
        params["firstname"] = map_at(data, "firstname");
        params["lastname"]  = map_at(data, "lastname");
        params["email"]     = map_at(data, "email");
        params["hostname"]  = map_at(data, "hostname");
        params["handle"]    = map_at(data, "handle");
        params["identification"] = map_at(data, "identification");
        params["passwd"]    = map_at(data, "pin1");

        Database::Connection conn = Database::Manager::acquire();

        /* for demo purpose we send second half of password as well */
        if (prai_ptr_->getPublicRequestManager()->getDemoMode() == true) {
            params["passwd2"] = map_at(data, "pin2");
            unsigned long long file_id = 0;

            Database::Result result = conn.exec_params(
                    "SELECT la.file_id FROM letter_archive la "
                    " JOIN message_archive ma ON ma.id=la.id "
                    " JOIN public_request_messages_map prmm ON prmm.message_archive_id = ma.id "
                    " WHERE prmm.public_request_id = $1::integer AND prmm.message_archive_id is not null",
                    Database::query_param_list(prai_ptr_->getId()));
            if (result.size() == 1) {
                file_id = result[0][0];
                attach.push_back(file_id);
            }
        }

        handles.push_back(prai_ptr_->getObject(0).handle);

        unsigned long long id = prai_ptr_->getPublicRequestManager()->getMailerManager()->sendEmail(
                "",           /* default sender */
                params["email"],
                "",           /* default subject */
                "mojeid_identification",
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


    void sendLetterPassword(const LetterType::Type &_type)
    {
        LOGGER(PACKAGE).debug("public request auth - send letter password");

        MessageData data = collectMessageData();

        std::stringstream xmldata, xml_part_code;
        Fred::Document::GenerationType doc_type;

        if (_type == LetterType::LETTER_PIN2) {
            xml_part_code << "<pin2>" << map_at(data, "pin2") << "</pin2>";
            doc_type = Fred::Document::GT_CONTACT_IDENTIFICATION_LETTER_PIN2;
        }
        else if (_type == LetterType::LETTER_PIN3) {
            xml_part_code << "<pin3>" << map_at(data, "pin3") << "</pin3>";
            doc_type = Fred::Document::GT_CONTACT_IDENTIFICATION_LETTER_PIN3;
        }
        else {
            throw std::runtime_error("unknown letter type");
        }

        std::string addr_country = ((map_at(data, "country_cs_name")).empty()
                ? map_at(data, "country_name")
                : map_at(data, "country_cs_name"));

        xmldata << "<?xml version='1.0' encoding='utf-8'?>"
                 << "<mojeid_auth>"
                 << "<user>"
                 << "<actual_date>" << map_at(data, "reqdate") << "</actual_date>"
                 << "<name>" << map_at(data, "firstname")
                             << " " << map_at(data, "lastname") << "</name>"
                 << "<organization>" << map_at(data, "organization") << "</organization>"
                 << "<street>" << map_at(data, "street") << "</street>"
                 << "<city>" << map_at(data, "city") << "</city>"
                 << "<stateorprovince>" << map_at(data, "stateorprovince") << "</stateorprovince>"
                 << "<postal_code>" << map_at(data, "postalcode") << "</postal_code>"
                 << "<country>" << addr_country << "</country>"
                 << "<account>"
                 << "<username>" << map_at(data, "handle") << "</username>"
                 << "<first_name>" << map_at(data, "firstname") << "</first_name>"
                 << "<last_name>" << map_at(data, "lastname") << "</last_name>"
                 << "<email>" << map_at(data, "email") << "</email>"
                 << "</account>"
                 << "<auth>"
                 << "<codes>"
                 << xml_part_code.str()
                 << "</codes>"
                 << "<link>" << map_at(data, "hostname") << "</link>"
                 << "</auth>"
                 << "</user>"
                 << "</mojeid_auth>";

            unsigned long long file_id = prai_ptr_->getPublicRequestManager()->getDocumentManager()->generateDocumentAndSave(
                doc_type,
                xmldata,
                "identification_request-" + boost::lexical_cast<std::string>(prai_ptr_->getId()) + ".pdf",
                7,
                "");

            Fred::Messages::PostalAddress pa;
            pa.name    = map_at(data, "firstname") + " " + map_at(data, "lastname");
            pa.org     = map_at(data, "organization");
            pa.street1 = map_at(data, "street");
            pa.street2 = std::string("");
            pa.street3 = std::string("");
            pa.city    = map_at(data, "city");
            pa.state   = map_at(data, "stateorprovince");
            pa.code    = map_at(data, "postalcode");
            pa.country = map_at(data, "country_name");

            unsigned long long message_id =
                    prai_ptr_->getPublicRequestManager()->getMessagesManager()->save_letter_to_send(
                    map_at(data, "handle").c_str()//contact handle
                    , pa
                    , file_id
                    , ((_type == LetterType::LETTER_PIN2) ? "mojeid_pin2"
                            : ((_type == LetterType::LETTER_PIN3) ? "mojeid_pin3" : "")) //message type
                    , boost::lexical_cast<unsigned long >(map_at(data, "contact_id"))//contact object_registry.id
                    , boost::lexical_cast<unsigned long >(map_at(data, "contact_hid"))//contact_history.historyid
                    , ((_type == LetterType::LETTER_PIN2) ? "registered_letter"
                            : ((_type == LetterType::LETTER_PIN3) ? "letter" : ""))//comm_type letter or registered_letter
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


    void sendSmsPassword()
    {
        LOGGER(PACKAGE).debug("public request auth - send sms password");

        MessageData data = collectMessageData();

        unsigned long long message_id =
                prai_ptr_->getPublicRequestManager()->getMessagesManager()->save_sms_to_send(
                map_at(data, "handle").c_str()
                , map_at(data, "phone").c_str()
                , (std::string("Potvrzujeme uspesne zalozeni uctu mojeID. "
                        "Pro aktivaci Vaseho uctu je nutne vlozit kody "
                        "PIN1 a PIN2. PIN1 Vam byl zaslan emailem, PIN2 je: ")
                 + map_at(data, "pin2")
                 ).c_str()
                , "mojeid_pin2"
                , boost::lexical_cast<unsigned long >(map_at(data, "contact_id"))
                , boost::lexical_cast<unsigned long >(map_at(data, "contact_hid"))
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
        if (rauthinfo.size() != 1) {
            throw std::runtime_error(str(boost::format(
                        "cannot retrieve authinfo for contact id=%1%")
                        % contact_id));
        }
        std::string passwd;
        /* pin1 */
        if (rauthinfo[0][0].isnull()) {
            passwd = generateRandomPassword(get_password_chunk_length());
        }
        else {
            passwd = static_cast<std::string>(rauthinfo[0][0]);
            LOGGER(PACKAGE).debug(boost::format("authinfo w/o spaces='%s'") % passwd);
            /* fill with random to PASSWORD_CHUNK_LENGTH size */
            size_t to_fill = 0;
            if ((to_fill = (get_password_chunk_length() - passwd.length())) > 0) {
                passwd += generateRandomPassword(to_fill);
                LOGGER(PACKAGE).debug(boost::format("authinfo filled='%s'") % passwd);
            }
        }
        /* append pin2 */
        passwd += generateRandomPassword(get_password_chunk_length());
        return passwd;
    }
};//class ContactVerificationPimpl

class ContactVerification : public Fred::PublicRequest::PublicRequestAuthImpl
{
    ContactVerificationPimpl contact_verification_impl_;
public:
    size_t get_password_chunk_length()
    {
        return contact_verification_impl_.get_password_chunk_length();
    }

    Fred::Contact::Verification::ContactValidator& get_contact_validator()
    {
        return contact_verification_impl_.get_contact_validator();
    }

    ContactVerification()
    : PublicRequestAuthImpl()
    , contact_verification_impl_(this)
    {}

    void sendEmailPassword(const EmailType::Type &_type)
    {
        contact_verification_impl_.sendEmailPassword(_type);
    }

    void sendLetterPassword(const LetterType::Type &_type)
    {
        contact_verification_impl_.sendLetterPassword(_type);
    }

    void sendSmsPassword()
    {
        contact_verification_impl_.sendSmsPassword();
    }

    std::string generateRandomPassword(const size_t _length)
    {
        return contact_verification_impl_.generateRandomPassword(_length);
    }

    std::string generateRandomPassword()
    {
        return contact_verification_impl_.generateRandomPassword();
    }

    std::string generateAuthInfoPassword()
    {
        return contact_verification_impl_.generateAuthInfoPassword();
    }
};

class ConditionalContactIdentificationPimpl
{
    ContactVerification* contact_verification_ptr_;
    Fred::Contact::Verification::ContactValidator& contact_validator_;
public:
    ConditionalContactIdentificationPimpl(
            ContactVerification* _contact_verification_ptr)
    : contact_verification_ptr_(_contact_verification_ptr)
    , contact_validator_(contact_verification_ptr_->get_contact_validator())
    {
        contact_validator_ = Fred::Contact::Verification
                ::create_conditional_identification_validator();
    }

    std::string generatePasswords()
    {
        if(contact_verification_ptr_->getPublicRequestManager()->getDemoMode())
        {
            return std::string(contact_verification_ptr_->get_password_chunk_length(),'1')//pin1:11111111
                +std::string(contact_verification_ptr_->get_password_chunk_length(),'2'); //pin2:22222222
        }
        else
        {
            return contact_verification_ptr_->generateAuthInfoPassword();
        }
    }

    void save()
    {
        /* insert */
        if (!contact_verification_ptr_->getId()) {
            Fred::Contact::Verification::Contact cdata
                = Fred::Contact::Verification::contact_info(
                        contact_verification_ptr_->getObject(0).id);
            contact_validator_.check(cdata);

            bool check_ok = true;

            /* contact prohibits operations:
             *   3 | serverTransferProhibited
             *   4 | serverUpdateProhibited
             */
            if (check_ok && (checkState(
                    contact_verification_ptr_->getObject(0).id, 3) == true)) {
                check_ok = false;
            }
            if (check_ok && (checkState(
                    contact_verification_ptr_->getObject(0).id, 4) == true)) {
                check_ok = false;
            }

            /* already CI */
            if (check_ok && (checkState(
                    contact_verification_ptr_->getObject(0).id, 21) == true)) {
                check_ok = false;
            }
            /* already I */
            if (check_ok && (checkState(
                    contact_verification_ptr_->getObject(0).id, 22) == true)) {
                check_ok = false;
            }
            /* already V */
            if (check_ok && (checkState(
                    contact_verification_ptr_->getObject(0).id, 23) == true)) {
                check_ok = false;
            }
            /* has V request */
            if (check_ok && (check_public_request(
                    contact_verification_ptr_->getObject(0).id,
                    PRT_CONTACT_VALIDATION) > 0)) {
                check_ok = false;
            }
            if (!check_ok) {
                throw NotApplicable("pre_insert_checks: failed!");
            }
            /* if there is another open CI close it */
            cancel_public_request(
                    contact_verification_ptr_->getObject(0).id,
                    PRT_CONDITIONAL_CONTACT_IDENTIFICATION,
                    contact_verification_ptr_->getRequestId());
            /* if there is another open I close it */
            cancel_public_request(
                    contact_verification_ptr_->getObject(0).id,
                    PRT_CONTACT_IDENTIFICATION,
                    contact_verification_ptr_->getRequestId());
        }
        contact_verification_ptr_->PublicRequestAuthImpl::save();
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(boost::format(
                "processing public request id=%1%")
                % contact_verification_ptr_->getId());

        /* object should not change */
        if (object_was_changed_since_request_create(
                contact_verification_ptr_->getId())) {
            throw ObjectChanged();
        }

        Fred::Contact::Verification::Contact cdata
            = Fred::Contact::Verification::contact_info(
                    contact_verification_ptr_->getObject(0).id);
        contact_validator_.check(cdata);

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        /* check if need to transfer and do so (TODO: make function (two copies) */
        Database::Result clid_result = conn.exec_params(
                "SELECT o.clid FROM object o JOIN contact c ON c.id = o.id"
                " WHERE c.id = $1::integer FOR UPDATE",
                Database::query_param_list(
                    contact_verification_ptr_->getObject(0).id));
        if (clid_result.size() != 1) {
            throw std::runtime_error("cannot find contact, object doesn't exist!?"
                    " (probably deleted?)");
        }
        unsigned long long act_registrar = static_cast<unsigned long long>(
            clid_result[0][0]);
        if (act_registrar != contact_verification_ptr_->getRegistrarId()) {
            /* run transfer command */
            ::MojeID::Request request(205
                , contact_verification_ptr_->getRegistrarId()
                , contact_verification_ptr_->getResolveRequestId());
            Fred::Contact::Verification::contact_transfer(
                    request.get_request_id(),
                    request.get_registrar_id(),
                    contact_verification_ptr_->getObject(0).id);
            Fred::Contact::Verification::contact_transfer_poll_message(
                    act_registrar, contact_verification_ptr_->getObject(0).id);
            request.end_success();
        }

        /* set state */
        insertNewStateRequest(contact_verification_ptr_->getId()
                , contact_verification_ptr_->getObject(0).id, 21);
        insertNewStateRequest(contact_verification_ptr_->getId()
                , contact_verification_ptr_->getObject(0).id, 24);

        /* prohibit operations on contact */
        if (checkState(contact_verification_ptr_->getObject(0).id, 1) == false)
        {
            /* set 1 | serverDeleteProhibited */
            insertNewStateRequest(contact_verification_ptr_->getId()
                    , contact_verification_ptr_->getObject(0).id, 1);
        }
        if (checkState(contact_verification_ptr_->getObject(0).id, 3) == false)
        {
            /* set 3 | serverTransferProhibited */
            insertNewStateRequest(contact_verification_ptr_->getId()
                    , contact_verification_ptr_->getObject(0).id, 3);
        }
        if (checkState(contact_verification_ptr_->getObject(0).id, 4) == false)
        {
            /* set 4 | serverUpdateProhibited */
            insertNewStateRequest(contact_verification_ptr_->getId()
                    , contact_verification_ptr_->getObject(0).id, 4);
        }

        /* update states */
        Fred::update_object_states(contact_verification_ptr_->getObject(0).id);

        /* make new request for finishing contact identification */
        PublicRequestAuthPtr new_request(dynamic_cast<PublicRequestAuth*>(
                contact_verification_ptr_->get_manager_ptr()->createRequest(
                        PRT_CONTACT_IDENTIFICATION)));
        if (new_request)
        {
            new_request->setRegistrarId(
                    contact_verification_ptr_->getRegistrarId());
            new_request->setRequestId(
                    contact_verification_ptr_->getResolveRequestId());
            new_request->addObject(
                    contact_verification_ptr_->getObject(0));
            new_request->save();
            new_request->sendPasswords();
        }

        tx.commit();
    }

    void sendPasswords()
    {
        contact_verification_ptr_->sendEmailPassword(EmailType::EMAIL_PIN2_SMS);
        contact_verification_ptr_->sendSmsPassword();
    }
};

class ConditionalContactIdentificationImpl
        : public ContactVerification,
          public Util::FactoryAutoRegister<PublicRequest
              , ConditionalContactIdentificationImpl>
{
            ConditionalContactIdentificationPimpl cond_contact_identification_impl;

public:
    ConditionalContactIdentificationImpl()
    : ContactVerification()
    , cond_contact_identification_impl(this)
    {}

    std::string generatePasswords()
    {
        return cond_contact_identification_impl.generatePasswords();
    }

    void save()
    {
        cond_contact_identification_impl.save();
    }

    void processAction(bool _check)
    {
        cond_contact_identification_impl.processAction(_check);
    }

    void sendPasswords()
    {
        cond_contact_identification_impl.sendPasswords();
    }


    static std::string registration_name()
    {
        return PRT_CONDITIONAL_CONTACT_IDENTIFICATION;
    }
};


class ContactIdentificationPimpl
{
    ContactVerification* contact_verification_ptr_;
    Fred::Contact::Verification::ContactValidator& contact_validator_;
public:
    ContactIdentificationPimpl(
        ContactVerification* _contact_verification_ptr)
    : contact_verification_ptr_(_contact_verification_ptr)
    , contact_validator_(contact_verification_ptr_->get_contact_validator())
    {
        contact_validator_ = Fred::Contact::Verification::create_identification_validator();
    }

    /* XXX: change validator in case contact is already CI */
    void addObject(const OID &_oid)
    {
        contact_verification_ptr_->PublicRequestImpl::addObject(_oid);

        if (checkState(contact_verification_ptr_->getObject(0).id, 21) == true)
        {
            contact_validator_ = Fred::Contact::Verification
                    ::create_finish_identification_validator();
        }
    }

    std::string generatePasswords()
    {
        if (checkState(contact_verification_ptr_->getObject(0).id, 21) == true)
        {
            /* generate pin3 */
            if(contact_verification_ptr_->getPublicRequestManager()
                    ->getDemoMode())
            {
                return std::string(contact_verification_ptr_
                        ->get_password_chunk_length(),'3');//pin3:33333333
            }
            else
            {
                return contact_verification_ptr_->generateRandomPassword();
            }
        }
        else {
            /* generate pin1 and pin2 */
            if(contact_verification_ptr_->getPublicRequestManager()
                    ->getDemoMode())
            {
                return std::string(contact_verification_ptr_
                        ->get_password_chunk_length(),'1')//pin1:11111111
                    +std::string(contact_verification_ptr_
                        ->get_password_chunk_length(),'2'); //pin2:22222222
            }
            else
            {
                return contact_verification_ptr_->generateAuthInfoPassword();
            }
        }
    }

    void save()
    {
        if (!contact_verification_ptr_->getId()) {
            Fred::Contact::Verification::Contact cdata
                = Fred::Contact::Verification::contact_info(
                        contact_verification_ptr_->getObject(0).id);
            contact_validator_.check(cdata);

            bool check_ok = true;

            /* don't check this when contact is already CI - we are creating
             * I request only for finishing identification - pin3 */
            if (check_ok && (checkState(contact_verification_ptr_
                    ->getObject(0).id, 21) == false)) {
                /* contact prohibits operations:
                 *   3 | serverTransferProhibited
                 *   4 | serverUpdateProhibited
                 */
                if (check_ok && (checkState(contact_verification_ptr_
                        ->getObject(0).id, 3) == true)) {
                    check_ok = false;
                }
                if (check_ok && (checkState(contact_verification_ptr_
                        ->getObject(0).id, 4) == true)) {
                    check_ok = false;
                }
            }

            /* already CI state and opened I reqeust (finishing identification
             * process with pin3 */
            if (check_ok && ((checkState(contact_verification_ptr_
                    ->getObject(0).id, 21) == true)
                        && (check_public_request(
                                contact_verification_ptr_->getObject(0).id,
                                PRT_CONTACT_IDENTIFICATION) > 0))) {
                check_ok = false;
            }
            /* already I */
            if (check_ok && (checkState(contact_verification_ptr_
                    ->getObject(0).id, 22) == true)) {
                check_ok = false;
            }
            /* already V */
            if (check_ok && (checkState(contact_verification_ptr_
                    ->getObject(0).id, 23) == true)) {
                check_ok = false;
            }
            /* has V request */
            if (check_ok && (check_public_request(
                    contact_verification_ptr_->getObject(0).id,
                        PRT_CONTACT_VALIDATION) > 0)) {
                check_ok = false;
            }
            if (!check_ok) {
                throw NotApplicable("pre_insert_checks: failed!");
            }
            /* if there is another open CI close it */
            cancel_public_request(
                    contact_verification_ptr_->getObject(0).id,
                    PRT_CONDITIONAL_CONTACT_IDENTIFICATION,
                    contact_verification_ptr_->getRequestId());
            /* if not state CI cancel I request */
            if (checkState(contact_verification_ptr_
                    ->getObject(0).id, 21) == false) {
                cancel_public_request(
                        contact_verification_ptr_->getObject(0).id,
                    PRT_CONTACT_IDENTIFICATION,
                    contact_verification_ptr_->getRequestId());
            }
        }
        contact_verification_ptr_->PublicRequestAuthImpl::save();
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(boost::format(
                "processing public request id=%1%")
        % contact_verification_ptr_->getId());

        /* object should not change */
        if (checkState(contact_verification_ptr_->getObject(0).id, 21) == false
                && object_was_changed_since_request_create(contact_verification_ptr_->getId())) {
            throw ObjectChanged();
        }

        Fred::Contact::Verification::Contact cdata
            = Fred::Contact::Verification::contact_info(
                    contact_verification_ptr_->getObject(0).id);
        contact_validator_.check(cdata);

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        /* check if need to transfer and do so (TODO: make function (two copies) */
        Database::Result clid_result = conn.exec_params(
                "SELECT o.clid FROM object o JOIN contact c ON c.id = o.id"
                " WHERE c.id = $1::integer FOR UPDATE",
                Database::query_param_list(
                        contact_verification_ptr_->getObject(0).id));
        if (clid_result.size() != 1) {
            throw std::runtime_error("cannot find contact, object doesn't exist!?"
                    " (probably deleted?)");
        }
        unsigned long long act_registrar = static_cast<unsigned long long>(clid_result[0][0]);
        if (act_registrar != contact_verification_ptr_->getRegistrarId()) {
            /* run transfer command */
            ::MojeID::Request request(205, contact_verification_ptr_->getRegistrarId()
                    , contact_verification_ptr_->getResolveRequestId());
            Fred::Contact::Verification::contact_transfer(
                    request.get_request_id(),
                    request.get_registrar_id(),
                    contact_verification_ptr_->getObject(0).id);
            Fred::Contact::Verification::contact_transfer_poll_message(
                    act_registrar, contact_verification_ptr_->getObject(0).id);
            request.end_success();
        }

        /* check if contact is already conditionally identified (21) and cancel state */
        Fred::cancel_object_state(contact_verification_ptr_->getObject(0).id
                , ::MojeID::CONDITIONALLY_IDENTIFIED_CONTACT);

        /* set new state */
        insertNewStateRequest(contact_verification_ptr_->getId()
                , contact_verification_ptr_->getObject(0).id, 22);

        if (checkState(contact_verification_ptr_->getObject(0).id, 24) == false) {
            insertNewStateRequest(contact_verification_ptr_->getId()
                    , contact_verification_ptr_->getObject(0).id, 24);
        }

        /* prohibit operations on contact */
        if (checkState(contact_verification_ptr_->getObject(0).id, 1) == false) {
            /* set 1 | serverDeleteProhibited */
            insertNewStateRequest(contact_verification_ptr_->getId()
                    , contact_verification_ptr_->getObject(0).id, 1);
        }
        if (checkState(contact_verification_ptr_->getObject(0).id, 3) == false) {
            /* set 3 | serverTransferProhibited */
            insertNewStateRequest(contact_verification_ptr_->getId()
                    , contact_verification_ptr_->getObject(0).id, 3);
        }
        if (checkState(contact_verification_ptr_->getObject(0).id, 4) == false) {
            /* set 4 | serverUpdateProhibited */
            insertNewStateRequest(contact_verification_ptr_->getId()
                    , contact_verification_ptr_->getObject(0).id, 4);
        }

        /* update states */
        Fred::update_object_states(contact_verification_ptr_->getObject(0).id);
        tx.commit();
    }

    void sendPasswords()
    {
        if (checkState(contact_verification_ptr_->getObject(0).id, 21) == true) {
            /* contact is already conditionally identified - send pin3 */
            contact_verification_ptr_->sendLetterPassword(LetterType::LETTER_PIN3);
            /* in demo mode we send pin3 as email attachment */
            if (contact_verification_ptr_->get_manager_ptr()->getDemoMode()) {
                contact_verification_ptr_->sendEmailPassword(EmailType::EMAIL_PIN2_LETTER);
            }
        }
        else {
            /* contact is fresh - send pin2 */
            contact_verification_ptr_->sendLetterPassword(LetterType::LETTER_PIN2);
            //email have letter in attachement in demo mode, so letter first
            contact_verification_ptr_->sendEmailPassword(EmailType::EMAIL_PIN2_LETTER);
        }
    }
};

class ContactIdentificationImpl
        : public ContactVerification,
          public Util::FactoryAutoRegister<PublicRequest, ContactIdentificationImpl>
{
            ContactIdentificationPimpl contact_identification_impl;
public:
    ContactIdentificationImpl()
    : ContactVerification()
    , contact_identification_impl(this)
    {}

    /* XXX: change validator in case contact is already CI */
    void addObject(const OID &_oid)
    {
        contact_identification_impl.addObject(_oid);
    }

    std::string generatePasswords()
    {
        return contact_identification_impl.generatePasswords();
    }

    void save()
    {
        contact_identification_impl.save();
    }

    void processAction(bool _check)
    {
        contact_identification_impl.processAction(_check);
    }

    void sendPasswords()
    {
        contact_identification_impl.sendPasswords();
    }

    static std::string registration_name()
    {
        return PRT_CONTACT_IDENTIFICATION;
    }
};



class ValidationRequestImpl
        : public PublicRequestImpl,
          public Util::FactoryAutoRegister<PublicRequest, ValidationRequestImpl>
{
public:
    bool check() const
    {
        return true;
    }

    void save()
    {
        if (!this->getId()) {
            bool check_ok = true;
            /* already CI */
            bool ci_state = (checkState(this->getObject(0).id, 21) == true);
            /* already I */
            bool i_state  = (checkState(this->getObject(0).id, 22) == true);
            if (check_ok && (!ci_state && !i_state)) {
                check_ok = false;
            }
            /* already V */
            if (check_ok && (checkState(this->getObject(0).id, 23) == true)) {
                check_ok = false;
            }
            if (!check_ok) {
                throw NotApplicable("pre_insert_checks: failed!");
            }

            /* has V request */
            if (check_ok && (check_public_request(
                        this->getObject(0).id,
                        PRT_CONTACT_VALIDATION) > 0)) {
                throw RequestExists(PRT_CONTACT_VALIDATION, this->getObject(0).id);
            }
        }
        PublicRequestImpl::save();
    }


    virtual std::string getTemplateName() const
    {
        return "mojeid_validation";
    }


    virtual void fillTemplateParams(Fred::Mailer::Parameters& params) const
    {
        params["reqdate"] = stringify(getCreateTime().date());
        params["reqid"] = stringify(getId());
        if (getObjectSize()) {
            params["type"] = stringify(getObject(0).type);
            params["handle"] = getObject(0).handle;
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
                Database::query_param_list(getId()));
        if (res.size() == 1) {
            params["name"] = std::string(res[0][0]);
            params["org"] = std::string(res[0][1]);
            params["ic"] = unsigned(res[0][3]) == 4 ? std::string(res[0][2])  : "";
            params["birthdate"] = (unsigned(res[0][3]) == 6
                    ? stringify(birthdate_from_string_to_date(res[0][2]))
                    : std::string(""));
            params["address"] = std::string(res[0][4]);
            params["status"] = getStatus() == PRS_ANSWERED ? "1" : "2";
        }
    }

    void invalidateAction()
    {
        LOGGER(PACKAGE).debug(boost::format(
                    "invalidation request id=%1%")
                    % this->getId());
        /* just send email - note that difference between succesfully
         * processed email and invalidated email is done
         * by setting status_ = PRS_INVALID which is passed to email in
         * fillTemplateParams(...) method -
         * (params["status"] = getStatus() == PRS_ANSWERED ? "1" : "2";)
         */
        answer_email_id_ = sendEmail();
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(boost::format(
                    "processing validation request id=%1%")
                    % getId());

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        if ((checkState(this->getObject(0).id, 21) == false) && checkState(this->getObject(0).id, 22) == false) {
            throw NotApplicable("cannot process contact validation: no identified state &&"
                    " no conditionally identified state");
        }

        /* check if contact is already conditionally identified (21) and cancel status */
        Fred::cancel_object_state(getObject(0).id, ::MojeID::CONDITIONALLY_IDENTIFIED_CONTACT);

        /* check if contact is already identified (22) and cancel status */
        if (Fred::cancel_object_state(getObject(0).id, ::MojeID::IDENTIFIED_CONTACT) == false) {
            /* otherwise there could be identification request */
            cancel_public_request(getObject(0).id, PRT_CONTACT_IDENTIFICATION, this->getResolveRequestId());
        }

        /* set new state */
        insertNewStateRequest(getId(), getObject(0).id, 23);
        Fred::update_object_states(getObject(0).id);
        tx.commit();
    }


    static std::string registration_name()
    {
        return PRT_CONTACT_VALIDATION;
    }
};


}
}

