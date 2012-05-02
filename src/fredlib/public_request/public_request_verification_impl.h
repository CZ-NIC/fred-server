#ifndef PUBLIC_REQUEST_VERIFICATION_IMPL_H_
#define PUBLIC_REQUEST_VERIFICATION_IMPL_H_

#include "public_request_impl.h"
#include "types/birthdate.h"
#include "types/stringify.h"
#include "object_states.h"
#include "mojeid/contact.h"
#include "mojeid/request.h"
#include "mojeid/mojeid_data_validation.h"
#include "mojeid/mojeid_contact_states.h"
#include "map_at.h"


namespace Fred {
namespace PublicRequest {


class ContactVerification : public PublicRequestAuthImpl
{
protected:
    ::MojeID::ContactValidator contact_validator_;
    static const size_t PASSWORD_CHUNK_LENGTH = 8;


    typedef std::map<std::string, std::string> MessageData;

    enum LetterType
    {
        LETTER_PIN2,
        LETTER_PIN3
    };

    enum EmailType
    {
        EMAIL_PIN2_SMS,
        EMAIL_PIN2_LETTER
    };


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
                Database::query_param_list(this->getObject(0).id));
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
        data["hostname"] = this->getPublicRequestManager()->getIdentificationMailAuthHostname();
        data["identification"] = this->getIdentification();
        data["handle"] = boost::algorithm::to_lower_copy(this->getObject(0).handle);
        /* password split */
        const std::string password = this->getPassword();
        data["pin1"] = password.substr(0, -PASSWORD_CHUNK_LENGTH + password.length());
        data["pin2"] = password.substr(-PASSWORD_CHUNK_LENGTH + password.length());
        data["pin3"] = password;
        data["reqdate"] = boost::gregorian::to_iso_extended_string(this->getCreateTime().date());
        data["contact_id"] = boost::lexical_cast<std::string>(this->getObject(0).id);
        data["contact_hid"] = static_cast<std::string>(result[0][8]);
        data["phone"] = static_cast<std::string>(result[0][9]);
        data["country_name"] = static_cast<std::string>(result[0][10]);
        data["country_cs_name"] = static_cast<std::string>(result[0][11]);

        return data;
    }


public:
    ContactVerification() : PublicRequestAuthImpl() { }


    void sendEmailPassword(const EmailType &_type)
    {
        LOGGER(PACKAGE).debug("public request auth - send email password");

        MessageData data = this->collectMessageData();

        Mailer::Attachments attach;
        Mailer::Handles handles;
        Mailer::Parameters params;

        unsigned short type = ((_type == EMAIL_PIN2_SMS) ? 1
                                : (_type == EMAIL_PIN2_LETTER) ? 2 : 0);
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
        if (this->getPublicRequestManager()->getDemoMode() == true) {
            params["passwd2"] = map_at(data, "pin2");
            unsigned long long file_id = 0;

            Database::Result result = conn.exec_params(
                    "SELECT la.file_id FROM letter_archive la "
                    " JOIN message_archive ma ON ma.id=la.id "
                    " JOIN public_request_messages_map prmm ON prmm.message_archive_id = ma.id "
                    " WHERE prmm.public_request_id = $1::integer AND prmm.message_archive_id is not null",
                    Database::query_param_list(this->getId()));
            if (result.size() == 1) {
                file_id = result[0][0];
                attach.push_back(file_id);
            }
        }

        handles.push_back(this->getObject(0).handle);

        unsigned long long id = this->getPublicRequestManager()->getMailerManager()->sendEmail(
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
                    (this->getId())
                    (Database::QPNull)
                    (id));
        tx.commit();
    }


    void sendLetterPassword(const LetterType &_type)
    {
        LOGGER(PACKAGE).debug("public request auth - send letter password");

        MessageData data = collectMessageData();

        std::stringstream xmldata, xml_part_code;
        Document::GenerationType doc_type;

        if (_type == LETTER_PIN2) {
            xml_part_code << "<pin2>" << map_at(data, "pin2") << "</pin2>";
            doc_type = Document::GT_CONTACT_IDENTIFICATION_LETTER_PIN2;
        }
        else if (_type == LETTER_PIN3) {
            xml_part_code << "<pin3>" << map_at(data, "pin3") << "</pin3>";
            doc_type = Document::GT_CONTACT_IDENTIFICATION_LETTER_PIN3;
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

            unsigned long long file_id = this->getPublicRequestManager()->getDocumentManager()->generateDocumentAndSave(
                doc_type,
                xmldata,
                "identification_request-" + boost::lexical_cast<std::string>(this->getId()) + ".pdf",
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
                this->getPublicRequestManager()->getMessagesManager()->save_letter_to_send(
                    map_at(data, "handle").c_str()//contact handle
                    , pa
                    , file_id
                    , ((_type == LETTER_PIN2) ? "mojeid_pin2"
                            : ((_type == LETTER_PIN3) ? "mojeid_pin3" : "")) //message type
                    , boost::lexical_cast<unsigned long >(map_at(data, "contact_id"))//contact object_registry.id
                    , boost::lexical_cast<unsigned long >(map_at(data, "contact_hid"))//contact_history.historyid
                    , ((_type == LETTER_PIN2) ? "registered_letter"
                            : ((_type == LETTER_PIN3) ? "letter" : ""))//comm_type letter or registered_letter
                    );

            Database::Connection conn = Database::Manager::acquire();
            Database::Transaction tx(conn);
            conn.exec_params("INSERT INTO public_request_messages_map "
                    " (public_request_id, message_archive_id, mail_archive_id) "
                    " VALUES ($1::integer, $2::integer, $3::integer)",
                    Database::query_param_list
                        (this->getId())
                        (message_id)
                        (Database::QPNull));
            tx.commit();
    }


    void sendSmsPassword()
    {
        LOGGER(PACKAGE).debug("public request auth - send sms password");

        MessageData data = collectMessageData();

        unsigned long long message_id =
            this->getPublicRequestManager()->getMessagesManager()->save_sms_to_send(
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
                    (this->getId())
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
        return generateRandomPassword(PASSWORD_CHUNK_LENGTH);
    }


    std::string generateAuthInfoPassword()
    {
        unsigned long long contact_id = this->getObject(0).id;

        Database::Connection conn = Database::Manager::acquire();
        Database::Result rauthinfo = conn.exec_params(
                "SELECT substr(replace(o.authinfopw, ' ', ''), 1, $1::integer) "
                " FROM object o JOIN contact c ON c.id = o.id"
                " WHERE c.id = $2::integer",
                Database::query_param_list(PASSWORD_CHUNK_LENGTH)
                                          (contact_id));
        if (rauthinfo.size() != 1) {
            throw std::runtime_error(str(boost::format(
                        "cannot retrieve authinfo for contact id=%1%")
                        % contact_id));
        }
        std::string passwd;
        /* pin1 */
        if (rauthinfo[0][0].isnull()) {
            passwd = generateRandomPassword(PASSWORD_CHUNK_LENGTH);
        }
        else {
            passwd = static_cast<std::string>(rauthinfo[0][0]);
            LOGGER(PACKAGE).debug(boost::format("authinfo w/o spaces='%s'") % passwd);
            /* fill with random to PASSWORD_CHUNK_LENGTH size */
            size_t to_fill = 0;
            if ((to_fill = (PASSWORD_CHUNK_LENGTH - passwd.length())) > 0) {
                passwd += generateRandomPassword(to_fill);
                LOGGER(PACKAGE).debug(boost::format("authinfo filled='%s'") % passwd);
            }
        }
        /* append pin2 */
        passwd += generateRandomPassword(PASSWORD_CHUNK_LENGTH);
        return passwd;
    }
};



class ConditionalContactIdentificationImpl : public ContactVerification
{
public:
    ConditionalContactIdentificationImpl() : ContactVerification()
    {
        contact_validator_ = ::MojeID::create_conditional_identification_validator();
    }


    std::string generatePasswords()
    {
        if(this->getPublicRequestManager()->getDemoMode())
        {
            return std::string(PASSWORD_CHUNK_LENGTH,'1')//pin1:11111111
                +std::string(PASSWORD_CHUNK_LENGTH,'2'); //pin2:22222222
        }
        else
        {
            return this->generateAuthInfoPassword();
        }
    }

    void save()
    {
        /* insert */
        if (!this->getId()) {
            ::MojeID::Contact cdata = ::MojeID::contact_info(this->getObject(0).id);
            contact_validator_.check(cdata);

            bool check_ok = true;

            /* contact prohibits operations:
             *   3 | serverTransferProhibited
             *   4 | serverUpdateProhibited
             */
            if (check_ok && (checkState(this->getObject(0).id, 3) == true)) {
                check_ok = false;
            }
            if (check_ok && (checkState(this->getObject(0).id, 4) == true)) {
                check_ok = false;
            }

            /* already CI */
            if (check_ok && (checkState(this->getObject(0).id, 21) == true)) {
                check_ok = false;
            }
            /* already I */
            if (check_ok && (checkState(this->getObject(0).id, 22) == true)) {
                check_ok = false;
            }
            /* already V */
            if (check_ok && (checkState(this->getObject(0).id, 23) == true)) {
                check_ok = false;
            }
            /* has V request */
            if (check_ok && (check_public_request(
                        this->getObject(0).id,
                        PRT_CONTACT_VALIDATION) > 0)) {
                check_ok = false;
            }
            if (!check_ok) {
                throw NotApplicable("pre_insert_checks: failed!");
            }
            /* if there is another open CI close it */
            cancel_public_request(
                    this->getObject(0).id,
                    PRT_CONDITIONAL_CONTACT_IDENTIFICATION,
                    this->getRequestId());
            /* if there is another open I close it */
            cancel_public_request(
                    this->getObject(0).id,
                    PRT_CONTACT_IDENTIFICATION,
                    this->getRequestId());
        }
        PublicRequestAuthImpl::save();
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(boost::format(
                "processing public request id=%1%")
                % getId());

        /* object should not change */
        if (object_was_changed_since_request_create(this->getId())) {
            throw ObjectChanged();
        }

        ::MojeID::Contact cdata = ::MojeID::contact_info(getObject(0).id);
        contact_validator_.check(cdata);

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        /* check if need to transfer and do so (TODO: make function (two copies) */
        Database::Result clid_result = conn.exec_params(
                "SELECT o.clid FROM object o JOIN contact c ON c.id = o.id"
                " WHERE c.id = $1::integer FOR UPDATE",
                Database::query_param_list(getObject(0).id));
        if (clid_result.size() != 1) {
            throw std::runtime_error("cannot find contact, object doesn't exist!?"
                    " (probably deleted?)");
        }
        unsigned long long act_registrar = static_cast<unsigned long long>(clid_result[0][0]);
        if (act_registrar != this->getRegistrarId()) {
            /* run transfer command */
            ::MojeID::Request request(205, this->getRegistrarId(), this->getResolveRequestId());
            ::MojeID::contact_transfer(
                    request.get_request_id(),
                    request.get_registrar_id(),
                    this->getObject(0).id);
            ::MojeID::contact_transfer_poll_message(act_registrar, this->getObject(0).id);
            request.end_success();
        }

        /* set state */
        insertNewStateRequest(getId(), getObject(0).id, 21);

        /* prohibit operations on contact */
        if (checkState(this->getObject(0).id, 1) == false) {
            /* set 1 | serverDeleteProhibited */
            insertNewStateRequest(getId(), getObject(0).id, 1);
        }
        if (checkState(this->getObject(0).id, 3) == false) {
            /* set 3 | serverTransferProhibited */
            insertNewStateRequest(getId(), getObject(0).id, 3);
        }
        if (checkState(this->getObject(0).id, 4) == false) {
            /* set 4 | serverUpdateProhibited */
            insertNewStateRequest(getId(), getObject(0).id, 4);
        }

        /* update states */
        Fred::update_object_states(getObject(0).id);

        /* make new request for finishing contact identification */
        PublicRequestAuthPtr new_request(dynamic_cast<PublicRequestAuth*>(
                man_->createRequest(PRT_CONTACT_IDENTIFICATION)));
        if (new_request) {
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
        this->sendEmailPassword(EMAIL_PIN2_SMS);
        this->sendSmsPassword();
    }
};


class ContactIdentificationImpl : public ContactVerification
{
public:
    ContactIdentificationImpl() : ContactVerification()
    {
        contact_validator_ = ::MojeID::create_identification_validator();
    }

    /* XXX: change validator in case contact is already CI */
    void addObject(const OID &_oid)
    {
        PublicRequestImpl::addObject(_oid);

        if (checkState(this->getObject(0).id, 21) == true) {
            contact_validator_ = ::MojeID::create_finish_identification_validator();
        }
    }

    std::string generatePasswords()
    {
        if (checkState(getObject(0).id, 21) == true) {
            /* generate pin3 */
            if(this->getPublicRequestManager()->getDemoMode())
            {
                return std::string(PASSWORD_CHUNK_LENGTH,'3');//pin3:33333333
            }
            else
            {
                return this->generateRandomPassword();
            }
        }
        else {
            /* generate pin1 and pin2 */
            if(this->getPublicRequestManager()->getDemoMode())
            {
                return std::string(PASSWORD_CHUNK_LENGTH,'1')//pin1:11111111
                    +std::string(PASSWORD_CHUNK_LENGTH,'2'); //pin2:22222222
            }
            else
            {
                return this->generateAuthInfoPassword();
            }
        }
    }

    void save()
    {
        if (!this->getId()) {
            ::MojeID::Contact cdata = ::MojeID::contact_info(this->getObject(0).id);
            contact_validator_.check(cdata);

            bool check_ok = true;

            /* don't check this when contact is already CI - we are creating
             * I request only for finishing identification - pin3 */
            if (check_ok && (checkState(this->getObject(0).id, 21) == false)) {
                /* contact prohibits operations:
                 *   3 | serverTransferProhibited
                 *   4 | serverUpdateProhibited
                 */
                if (check_ok && (checkState(this->getObject(0).id, 3) == true)) {
                    check_ok = false;
                }
                if (check_ok && (checkState(this->getObject(0).id, 4) == true)) {
                    check_ok = false;
                }
            }

            /* already CI state and opened I reqeust (finishing identification
             * process with pin3 */
            if (check_ok && ((checkState(this->getObject(0).id, 21) == true)
                        && (check_public_request(
                                this->getObject(0).id,
                                PRT_CONTACT_IDENTIFICATION) > 0))) {
                check_ok = false;
            }
            /* already I */
            if (check_ok && (checkState(this->getObject(0).id, 22) == true)) {
                check_ok = false;
            }
            /* already V */
            if (check_ok && (checkState(this->getObject(0).id, 23) == true)) {
                check_ok = false;
            }
            /* has V request */
            if (check_ok && (check_public_request(
                        this->getObject(0).id,
                        PRT_CONTACT_VALIDATION) > 0)) {
                check_ok = false;
            }
            if (!check_ok) {
                throw NotApplicable("pre_insert_checks: failed!");
            }
            /* if there is another open CI close it */
            cancel_public_request(
                    this->getObject(0).id,
                    PRT_CONDITIONAL_CONTACT_IDENTIFICATION,
                    this->getRequestId());
            /* if not state CI cancel I request */
            if (checkState(this->getObject(0).id, 21) == false) {
                cancel_public_request(
                    this->getObject(0).id,
                    PRT_CONTACT_IDENTIFICATION,
                    this->getRequestId());
            }
        }
        PublicRequestAuthImpl::save();
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(boost::format(
                "processing public request id=%1%") % getId());

        /* object should not change */
        if (checkState(this->getObject(0).id, 21) == false
                && object_was_changed_since_request_create(this->getId())) {
            throw ObjectChanged();
        }

        ::MojeID::Contact cdata = ::MojeID::contact_info(getObject(0).id);
        contact_validator_.check(cdata);

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        /* check if need to transfer and do so (TODO: make function (two copies) */
        Database::Result clid_result = conn.exec_params(
                "SELECT o.clid FROM object o JOIN contact c ON c.id = o.id"
                " WHERE c.id = $1::integer FOR UPDATE",
                Database::query_param_list(getObject(0).id));
        if (clid_result.size() != 1) {
            throw std::runtime_error("cannot find contact, object doesn't exist!?"
                    " (probably deleted?)");
        }
        unsigned long long act_registrar = static_cast<unsigned long long>(clid_result[0][0]);
        if (act_registrar != this->getRegistrarId()) {
            /* run transfer command */
            ::MojeID::Request request(205, this->getRegistrarId(), this->getResolveRequestId());
            ::MojeID::contact_transfer(
                    request.get_request_id(),
                    request.get_registrar_id(),
                    this->getObject(0).id);
            ::MojeID::contact_transfer_poll_message(act_registrar, this->getObject(0).id);
            request.end_success();
        }

        /* check if contact is already conditionally identified (21) and cancel state */
        Fred::cancel_object_state(getObject(0).id, ::MojeID::CONDITIONALLY_IDENTIFIED_CONTACT);

        /* set new state */
        insertNewStateRequest(getId(), getObject(0).id, 22);

        /* prohibit operations on contact */
        if (checkState(this->getObject(0).id, 1) == false) {
            /* set 1 | serverDeleteProhibited */
            insertNewStateRequest(getId(), getObject(0).id, 1);
        }
        if (checkState(this->getObject(0).id, 3) == false) {
            /* set 3 | serverTransferProhibited */
            insertNewStateRequest(getId(), getObject(0).id, 3);
        }
        if (checkState(this->getObject(0).id, 4) == false) {
            /* set 4 | serverUpdateProhibited */
            insertNewStateRequest(getId(), getObject(0).id, 4);
        }

        /* update states */
        Fred::update_object_states(getObject(0).id);
        tx.commit();
    }

    void sendPasswords()
    {
        if (checkState(getObject(0).id, 21) == true) {
            /* contact is already conditionally identified - send pin3 */
            this->sendLetterPassword(LETTER_PIN3);
            /* in demo mode we send pin3 as email attachment */
            if (man_->getDemoMode()) {
                this->sendEmailPassword(EMAIL_PIN2_LETTER);
            }
        }
        else {
            /* contact is fresh - send pin2 */
            this->sendLetterPassword(LETTER_PIN2);
            //email have letter in attachement in demo mode, so letter first
            this->sendEmailPassword(EMAIL_PIN2_LETTER);
        }
    }
};



class ValidationRequestImpl : public PublicRequestImpl
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


    virtual void fillTemplateParams(Mailer::Parameters& params) const
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
};


}
}

#endif /* PUBLIC_REQUEST_VERIFICATION_IMPL_H_ */

