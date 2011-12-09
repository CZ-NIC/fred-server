#ifndef PUBLIC_REQUEST_VERIFICATION_IMPL_H_
#define PUBLIC_REQUEST_VERIFICATION_IMPL_H_

#include "public_request_impl.h"
#include "types/birthdate.h"
#include "object_states.h"
#include "mojeid/contact.h"
#include "mojeid/request.h"
#include "mojeid/mojeid_data_validation.h"
#include "mojeid/mojeid_contact_states.h"


namespace Fred {
namespace PublicRequest {


class ConditionalContactIdentificationImpl
    : public PublicRequestAuthImpl
{
public:
    ConditionalContactIdentificationImpl() : PublicRequestAuthImpl()
    {
        contact_validator_ = ::MojeID::create_conditional_identification_validator();
    }

    std::string generatePasswords()
    {
        return this->generateAuthInfoPassword();
    }

    void save()
    {
        /* insert */
        if (!this->getId()) {
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
                    request.get_id(),
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
            new_request->setEppActionId(this->getEppActionId());
            new_request->addObject(this->getObject(0));
            new_request->save();
            new_request->sendPasswords();
        }

        tx.commit();
    }

    void sendPasswords()
    {
        MessageData data = PublicRequestAuthImpl::collectMessageData();
        PublicRequestAuthImpl::sendEmailPassword(data, 1);
        PublicRequestAuthImpl::sendSmsPassword(data);
    }
};


class ContactIdentificationImpl
  : public PublicRequestAuthImpl
{
public:
    ContactIdentificationImpl() : PublicRequestAuthImpl()
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
            return this->generateRandomPassword(PASSWORD_CHUNK_LENGTH);
        }
        else {
            /* generate pin1 and pin2 */
            return this->generateAuthInfoPassword();
        }
    }

    void save()
    {
        if (!this->getId()) {
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
                    request.get_id(),
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
        MessageData data = PublicRequestAuthImpl::collectMessageData();

        if (checkState(getObject(0).id, 21) == true) {
            /* contact is already conditionally identified - send pin3 */
            PublicRequestAuthImpl::sendLetterPassword(data, LETTER_PIN3);
            /* in demo mode we send pin3 as email attachment */
            if (man_->getDemoMode()) {
                PublicRequestAuthImpl::sendEmailPassword(data, 2);
            }
        }
        else {
            /* contact is fresh - send pin2 */
            PublicRequestAuthImpl::sendLetterPassword(data, LETTER_PIN2);
            //email have letter in attachement in demo mode, so letter first
            PublicRequestAuthImpl::sendEmailPassword(data, 2);
        }
    }
};



class ValidationRequestImpl
    : public PublicRequestImpl
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

