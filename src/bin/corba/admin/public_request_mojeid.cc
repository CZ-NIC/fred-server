#include "src/bin/corba/admin/public_request_mojeid.hh"

#include "src/libfred/contact_verification/contact_verification_state.hh"
#include "src/libfred/mailer.hh"
#include "src/libfred/object_states.hh"
#include "src/libfred/object_states.hh"
#include "src/libfred/public_request/public_request_impl.hh"
#include "src/util/types/birthdate.hh"

namespace CorbaConversion {
namespace Admin {

FACTORY_MODULE_INIT_DEFI(mojeid)

class MojeIdPublicRequestBase
        : public LibFred::PublicRequest::PublicRequestAuthImpl
{
public:
    void sendPasswords()
    {
        throw std::runtime_error(std::string("method ") + __PRETTY_FUNCTION__ + " should never be called");
    }

    std::string generatePasswords()
    {
        throw std::runtime_error(std::string("method ") + __PRETTY_FUNCTION__ + " should never be called");
    }
};

class MojeIdCCI
        : public MojeIdPublicRequestBase,
          public Util::FactoryAutoRegister<LibFred::PublicRequest::PublicRequest, MojeIdCCI>
{
public:
    static std::string registration_name()
    {
        return CorbaConversion::Admin::PRT_MOJEID_CONTACT_CONDITIONAL_IDENTIFICATION;
    }
};

class MojeIdCI
        : public MojeIdPublicRequestBase,
          public Util::FactoryAutoRegister<LibFred::PublicRequest::PublicRequest, MojeIdCI>
{
public:
    static std::string registration_name()
    {
        return CorbaConversion::Admin::PRT_MOJEID_CONTACT_IDENTIFICATION;
    }
};

class MojeIdCICT
        : public MojeIdPublicRequestBase,
          public Util::FactoryAutoRegister<LibFred::PublicRequest::PublicRequest, MojeIdCICT>
{
public:
    static std::string registration_name()
    {
        return CorbaConversion::Admin::PRT_MOJEID_CONDITIONALLY_IDENTIFIED_CONTACT_TRANSFER;
    }
};

class MojeIdICT
        : public MojeIdPublicRequestBase,
          public Util::FactoryAutoRegister<LibFred::PublicRequest::PublicRequest, MojeIdICT>
{
public:
    static std::string registration_name()
    {
        return CorbaConversion::Admin::PRT_MOJEID_IDENTIFIED_CONTACT_TRANSFER;
    }
};

class MojeIdCRI
        : public MojeIdPublicRequestBase,
          public Util::FactoryAutoRegister<LibFred::PublicRequest::PublicRequest, MojeIdCRI>
{
public:
    static std::string registration_name()
    {
        return CorbaConversion::Admin::PRT_MOJEID_CONTACT_REIDENTIFICATION;
    }
};

class MojeIdCPUT
        : public MojeIdPublicRequestBase,
          public Util::FactoryAutoRegister<LibFred::PublicRequest::PublicRequest, MojeIdCPUT>
{
public:
    static std::string registration_name()
    {
        return CorbaConversion::Admin::PRT_MOJEID_CONTACT_PREVALIDATED_UNIDENTIFIED_TRANSFER;
    }
};

class MojeIdCPT
        : public MojeIdPublicRequestBase,
          public Util::FactoryAutoRegister<LibFred::PublicRequest::PublicRequest, MojeIdCPT>
{
public:
    static std::string registration_name()
    {
        return CorbaConversion::Admin::PRT_MOJEID_CONTACT_PREVALIDATED_TRANSFER;
    }
};

class MojeIdCV
        : public LibFred::PublicRequest::PublicRequestImpl,
          public Util::FactoryAutoRegister<LibFred::PublicRequest::PublicRequest, MojeIdCV>
{
public:
    static std::string registration_name()
    {
        return CorbaConversion::Admin::PRT_MOJEID_CONTACT_VALIDATION;
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
         * (params["status"] = getStatus() == PRS_RESOLVED ? "1" : "2";)
         */
        this->get_answer_email_id() = this->sendEmail();
    }

    void processAction(bool _check)
    {
        LOGGER(PACKAGE).debug(boost::format("processing validation request id=%1%") % this->getId());

        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);

        const unsigned long long oid = this->getObject(0).id;

        const LibFred::Contact::Verification::State contact_state =
                LibFred::Contact::Verification::get_contact_verification_state(oid);
        if (contact_state.has_all(LibFred::Contact::Verification::State::ciVm) ||
                !contact_state.has_all(LibFred::Contact::Verification::State::CivM))
        {
            throw LibFred::PublicRequest::NotApplicable("pre_insert_checks: failed!");
        }

        /* set new state */
        LibFred::PublicRequest::insertNewStateRequest(this->getId(), oid, "validatedContact");
        LibFred::update_object_states(oid);

        tx.commit();
    }

    void fillTemplateParams(::LibFred::Mailer::Parameters& params) const
    {
        params["reqdate"] = stringify(this->getCreateTime().date());
        params["reqid"] = stringify(this->getId());

        if (this->getObjectSize())
        {
            params["type"] = stringify(this->getObject(0).type);
            params["handle"] = this->getObject(0).handle;
        }

        Database::Connection conn = Database::Manager::acquire();
        Database::Result res = conn.exec_params(
                // clang-format off
                "SELECT c.name,c.organization,c.ssn,"
                       "(SELECT type FROM enum_ssntype WHERE id=c.ssntype),"
                       "CONCAT_WS(', ',NULLIF(BTRIM(c.street1),''),NULLIF(BTRIM(c.street2),''),"
                                      "NULLIF(BTRIM(c.street3),''),NULLIF(BTRIM(c.postalcode),''),"
                                      "NULLIF(BTRIM(c.city),''),c.country) "
                "FROM public_request pr "
                "JOIN public_request_objects_map prom ON prom.request_id=pr.id "
                "JOIN contact c ON c.id=prom.object_id "
                "WHERE pr.id=$1::INTEGER", Database::query_param_list(this->getId()));
                // clang-format on
        if (res.size() == 1)
        {
            const std::string ssn = res[0][2].isnull() ? std::string()
                                                       : static_cast<std::string>(res[0][2]);
            const std::string ssn_type = res[0][3].isnull() ? std::string()
                                                            : static_cast<std::string>(res[0][3]);
            params["name"] = res[0][0].isnull() ? std::string() : static_cast<std::string>(res[0][0]);
            params["org"] = res[0][1].isnull() ? std::string() : static_cast<std::string>(res[0][1]);
            params["ic"] = ssn_type == "ICO" ? ssn : std::string();
            params["birthdate"] = (ssn_type == "BIRTHDAY") && !ssn.empty()
                                          ? stringify(birthdate_from_string_to_date(ssn))
                                          : std::string();
            params["address"] = res[0][4].isnull() ? std::string() : static_cast<std::string>(res[0][4]);
            params["status"] = this->getStatus() == LibFred::PublicRequest::PRS_RESOLVED ? "1" : "2";
        }
    }

    std::string getTemplateName() const
    {
        return "mojeid_validation";
    }

    void save()
    {
        if (this->getId() == 0)
        {
            throw std::runtime_error("insert new request disabled");
        }
        PublicRequestImpl::save();
    }
};

} // namespace CorbaConversion::Admin
} // namespace CorbaConversion
