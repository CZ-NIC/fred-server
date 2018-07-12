#include "src/libfred/public_request/public_request_personal_info_impl.hh"
#include "src/libfred/public_request/public_request_impl.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"

#include <string>

namespace LibFred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DEFI(personal_info)


const Type PRT_PERSONALINFO_AUTO_PIF = "personalinfo_auto_pif";
const Type PRT_PERSONALINFO_EMAIL_PIF = "personalinfo_email_pif";
const Type PRT_PERSONALINFO_POST_PIF = "personalinfo_post_pif";
const Type PRT_PERSONALINFO_GOVERNMENT_PIF = "personalinfo_government_pif";

class PersonalInfoRequestImpl
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


class PersonalInfoRequestPIFAutoImpl
        : public PersonalInfoRequestImpl,
          public Util::FactoryAutoRegister<PublicRequest, PersonalInfoRequestPIFAutoImpl>
{
public:
    static std::string registration_name()
    {
        return PRT_PERSONALINFO_AUTO_PIF;
    }
};


class PersonalInfoRequestPIFEmailImpl
        : public PersonalInfoRequestImpl,
          public Util::FactoryAutoRegister<PublicRequest, PersonalInfoRequestPIFEmailImpl>
{
public:
    static std::string registration_name()
    {
        return PRT_PERSONALINFO_EMAIL_PIF;
    }
};


class PersonalInfoRequestPIFPostImpl
        : public PersonalInfoRequestImpl,
          public Util::FactoryAutoRegister<PublicRequest, PersonalInfoRequestPIFPostImpl>
{
public:
    static std::string registration_name()
    {
        return PRT_PERSONALINFO_POST_PIF;
    }
};


class PersonalInfoRequestPIFGovernmentImpl
        : public PersonalInfoRequestImpl,
          public Util::FactoryAutoRegister<PublicRequest, PersonalInfoRequestPIFGovernmentImpl>
{
public:
    static std::string registration_name()
    {
        return PRT_PERSONALINFO_GOVERNMENT_PIF;
    }
};


} // namespace LibFred::PublicRequest
} // namespace LibFred
