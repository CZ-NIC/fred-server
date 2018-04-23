#include <string>

#include "src/libfred/public_request/public_request_personalinfo_impl.hh"
#include "src/libfred/public_request/public_request_impl.hh"


namespace LibFred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DEFI(personalinfo)


const Type PRT_PERSONALINFO_AUTO_PIF = "personalinfo_auto_pif";
const Type PRT_PERSONALINFO_EMAIL_PIF = "personalinfo_email_pif";
const Type PRT_PERSONALINFO_POST_PIF = "personalinfo_post_pif";

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


}
}


