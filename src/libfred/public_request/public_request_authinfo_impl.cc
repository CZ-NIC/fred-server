#include "src/libfred/public_request/public_request_authinfo_impl.hh"

#include "src/libfred/public_request/public_request_impl.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"

#include <string>

namespace LibFred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DEFI(authinfo)


const Type PRT_AUTHINFO_AUTO_RIF = "authinfo_auto_rif";
const Type PRT_AUTHINFO_AUTO_PIF = "authinfo_auto_pif";
const Type PRT_AUTHINFO_EMAIL_PIF = "authinfo_email_pif";
const Type PRT_AUTHINFO_POST_PIF = "authinfo_post_pif";
const Type PRT_AUTHINFO_GOVERNMENT_PIF = "authinfo_government_pif";

class AuthInfoRequestImpl
        : public PublicRequestImpl
{
public:
    bool check() const
    {
        return true;
    }

    std::string getTemplateName() const
    {
        return std::string();
    }

    void fillTemplateParams(LibFred::Mailer::Parameters&) const
    {
    }

    TID sendEmail() const
    {
        return 0;
    }

    void save()
    {
        if (this->getId() == 0)
        {
            throw std::runtime_error("insert new request disabled");
        }
        PublicRequestImpl::save();
        Database::Connection conn = Database::Manager::acquire();
        conn.exec_params(
            "UPDATE public_request SET on_status_action = $1::enum_on_status_action_type"
            " WHERE id = $2::bigint",
            Database::query_param_list
                (Conversion::Enums::to_db_handle(OnStatusAction::scheduled))
                (this->getId())
        );
    }
};


class AuthInfoRequestPIFAutoImpl
        : public AuthInfoRequestImpl,
          public Util::FactoryAutoRegister<PublicRequest, AuthInfoRequestPIFAutoImpl>
{
public:
    static std::string registration_name()
    {
        return PRT_AUTHINFO_AUTO_PIF;
    }
};


class AuthInfoRequestPIFEmailImpl
        : public AuthInfoRequestImpl,
          public Util::FactoryAutoRegister<PublicRequest, AuthInfoRequestPIFEmailImpl>
{
public:
    static std::string registration_name()
    {
        return PRT_AUTHINFO_EMAIL_PIF;
    }
};


class AuthInfoRequestPIFPostImpl
        : public AuthInfoRequestImpl,
          public Util::FactoryAutoRegister<PublicRequest, AuthInfoRequestPIFPostImpl>
{
public:
    static std::string registration_name()
    {
        return PRT_AUTHINFO_POST_PIF;
    }
};


class AuthInfoRequestPIFGovernmentImpl
        : public AuthInfoRequestImpl,
          public Util::FactoryAutoRegister<PublicRequest, AuthInfoRequestPIFGovernmentImpl>
{
public:
    static std::string registration_name()
    {
        return PRT_AUTHINFO_GOVERNMENT_PIF;
    }
};


} // namespace LibFred::PublicRequest
} // namespace LibFred
