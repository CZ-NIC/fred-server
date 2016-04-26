#include "src/corba/admin/public_request_mojeid.h"
#include "src/fredlib/public_request/public_request_impl.h"
#include "src/fredlib/contact_verification/contact_verification_state.h"

#include "types/birthdate.h"
#include "src/fredlib/object_states.h"
#include "src/fredlib/mailer.h"
#include "src/fredlib/object_states.h"

namespace Fred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DEFI(mojeid)

class MojeIDPublicRequestBase
:   public Fred::PublicRequest::PublicRequestAuthImpl
{
public:
    void sendPasswords()
    {
        throw std::runtime_error(std::string("method ") + __PRETTY_FUNCTION__ + " has never to be called");
    }

    std::string generatePasswords()
    {
        throw std::runtime_error(std::string("method ") + __PRETTY_FUNCTION__ + " has never to be called");
    }
};

class MojeIDCCI
:   public MojeIDPublicRequestBase,
    public Util::FactoryAutoRegister< PublicRequest, MojeIDCCI >
{
public:
    static std::string registration_name()
    {
        return PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION;
    }
};

class MojeIDCI
:   public MojeIDPublicRequestBase,
    public Util::FactoryAutoRegister< PublicRequest, MojeIDCI >
{
public:
    static std::string registration_name()
    {
        return PRT_MOJEID_CONTACT_IDENTIFICATION;
    }
};

class MojeIDCICT
:   public MojeIDPublicRequestBase,
    public Util::FactoryAutoRegister< PublicRequest, MojeIDCICT >
{
public:
    static std::string registration_name()
    {
        return PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER;
    }
};

class MojeIDICT
:   public MojeIDPublicRequestBase,
    public Util::FactoryAutoRegister< PublicRequest, MojeIDICT >
{
public:
    static std::string registration_name()
    {
        return PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER;
    }
};

class MojeIDCRI
:   public MojeIDPublicRequestBase,
    public Util::FactoryAutoRegister< PublicRequest, MojeIDCRI >
{
public:
    static std::string registration_name()
    {
        return PRT_MOJEID_CONTACT_REIDENTIFICATION;
    }
};

class MojeIDCPUT
:   public MojeIDPublicRequestBase,
    public Util::FactoryAutoRegister< PublicRequest, MojeIDCPUT >
{
public:
    static std::string registration_name()
    {
        return PRT_MOJEID_CONTACT_PREVALIDATED_UNIDENTIFIED_TRANSFER;
    }
};

class MojeIDCPT
:   public MojeIDPublicRequestBase,
    public Util::FactoryAutoRegister< PublicRequest, MojeIDCPT >
{
public:
    static std::string registration_name()
    {
        return PRT_MOJEID_CONTACT_PREVALIDATED_TRANSFER;
    }
};

class MojeIDCV
:   public PublicRequestImpl,
    public Util::FactoryAutoRegister< PublicRequest, MojeIDCV >
{
public:
    static std::string registration_name()
    {
        return PRT_MOJEID_CONTACT_VALIDATION;
    }

    bool check() const
    {
        return true;
    }

    void invalidateAction()
    {
        LOGGER(PACKAGE).debug(boost::format("invalidation request id=%1%") % this->getId());
        /* just send email - note that difference between succesfully
         * processed email and invalidated email is done
         * by setting status_ = PRS_INVALID which is passed to email in
         * fillTemplateParams(...) method -
         * (params["status"] = getStatus() == PRS_ANSWERED ? "1" : "2";)
         */
        this->get_answer_email_id() = this->sendEmail();
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(boost::format("processing validation request id=%1%") % this->getId());

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        const unsigned long long oid = this->getObject(0).id;

        const Contact::Verification::State contact_state =
            Contact::Verification::get_contact_verification_state(oid);
        if (contact_state.has_all(Contact::Verification::State::ciVm) ||
           !contact_state.has_all(Contact::Verification::State::CivM)) {
            throw NotApplicable("pre_insert_checks: failed!");
        }

        /* set new state */
        insertNewStateRequest(this->getId(), oid, "validatedContact");
        ::Fred::update_object_states(oid);
        tx.commit();
    }

    void fillTemplateParams(::Fred::Mailer::Parameters &params)const
    {
        params["reqdate"] = stringify(this->getCreateTime().date());
        params["reqid"]   = stringify(this->getId());

        if (this->getObjectSize()) {
            params["type"]   = stringify(this->getObject(0).type);
            params["handle"] = this->getObject(0).handle;
        }

        Database::Connection conn = Database::Manager::acquire();
        Database::Result res = conn.exec_params(
            "SELECT c.name,c.organization,c.ssn,"
                   "(SELECT type FROM enum_ssntype WHERE id=c.ssntype),"
                   "CONCAT_WS(', ',NULLIF(BTRIM(c.street1),''),NULLIF(BTRIM(c.street2),''),"
                                  "NULLIF(BTRIM(c.street3),''),NULLIF(BTRIM(c.postalcode),''),"
                                  "NULLIF(BTRIM(c.city),''),c.country) "
            "FROM public_request pr "
            "JOIN public_request_objects_map prom ON prom.request_id=pr.id "
            "JOIN contact c ON c.id=prom.object_id "
            "WHERE pr.id=$1::INTEGER", Database::query_param_list(this->getId()));
        if (res.size() == 1) {
            const std::string ssn      = res[0][2].isnull() ? std::string()
                                                            : static_cast< std::string >(res[0][2]);
            const std::string ssn_type = res[0][3].isnull() ? std::string()
                                                            : static_cast< std::string >(res[0][3]);
            params["name"]      = res[0][0].isnull() ? std::string() : static_cast< std::string >(res[0][0]);
            params["org"]       = res[0][1].isnull() ? std::string() : static_cast< std::string >(res[0][1]);
            params["ic"]        = ssn_type == "ICO" ? ssn : std::string();
            params["birthdate"] = (ssn_type == "BIRTHDAY") && !ssn.empty()
                                  ? stringify(birthdate_from_string_to_date(ssn))
                                  : std::string();
            params["address"]   = res[0][4].isnull() ? std::string() : static_cast< std::string >(res[0][4]);
            params["status"]    = this->getStatus() == PRS_ANSWERED ? "1" : "2";
        }
    }

    std::string getTemplateName()const
    {
        return "mojeid_validation";
    }

    void save()
    {
        if (this->getId() == 0) {
            throw std::runtime_error("insert new request disabled");
        }
        PublicRequestImpl::save();
    }
};

}
}
