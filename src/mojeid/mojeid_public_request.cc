#include "src/mojeid/mojeid_public_request.h"

namespace Fred {
namespace MojeID {
namespace PublicRequest {

std::string ContactConditionalIdentification::get_public_request_type()const
{
    static const std::string type = "mojeid_contact_conditional_identification";
    return type;
}

std::string ContactConditionalIdentification::generate_passwords()const
{
    return "";
}

}//Fred::MojeID::PublicRequest
}//Fred::MojeID
}//Fred
