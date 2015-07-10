#include "src/mojeid/mojeid_public_request.h"
#include "util/random.h"
#include "util/cfg/handle_contactverification_args.h"
#include "util/cfg/handle_mojeid_args.h"
#include "util/cfg/config_handler_decl.h"

namespace Fred {

namespace Password {

enum { CHUNK_LENGTH = 8 };

std::string generate(::size_t _length = CHUNK_LENGTH)
{
    static const std::string set_of_possible_values = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789";
    return Random::string_from(_length, set_of_possible_values);
}

}//Fred::Password

namespace {

std::string get_demo_pin1()//11111111
{
    static const std::string pin1(Password::CHUNK_LENGTH, '1');
    return pin1;
}

std::string get_demo_pin2()//22222222
{
    static const std::string pin2(Password::CHUNK_LENGTH, '2');
    return pin2;
}

std::string get_demo_pin1_pin2()
{
    static const std::string pin1_pin2 = get_demo_pin1() +
                                         get_demo_pin2();
    return pin1_pin2;
}

std::string generate_pin()
{
    return Password::generate();
}

std::string generate_pin1()
{
    return generate_pin();
}

std::string generate_pin2()
{
    return generate_pin();
}

std::string generate_pin1_pin2()
{
    return generate_pin1() + generate_pin2();
}

}//Fred::{anonymous}

std::string conditional_contact_identification_generate_passwords()
{
    static const bool runs_in_demo_mode =
        CfgArgs::instance()->get_handler_ptr_by_type< HandleContactVerificationArgs >()->demo_mode;

    return runs_in_demo_mode ? get_demo_pin1_pin2()
                             : generate_pin1_pin2();
}

namespace MojeID {

std::string contact_transfer_request_generate_passwords()
{
    static const bool runs_in_demo_mode =
        CfgArgs::instance()->get_handler_ptr_by_type< HandleMojeIDArgs >()->demo_mode;

    return runs_in_demo_mode ? get_demo_pin1()
                             : generate_pin1();
}

namespace PublicRequest {

std::string ContactConditionalIdentification::get_public_request_type()const
{
    static const std::string type = "mojeid_contact_conditional_identification";
    return type;
}

std::string ContactConditionalIdentification::generate_passwords()const
{
    const std::string cci_pass = conditional_contact_identification_generate_passwords();
    const std::string mtr_pass = contact_transfer_request_generate_passwords();
    /* merge transfer pin with cond. contact identification */
    return mtr_pass + cci_pass.substr(mtr_pass.length());
}

std::string ConditionallyIdentifiedContactTransfer::get_public_request_type()const
{
    static const std::string type = "mojeid_conditionally_identified_contact_transfer";
    return type;
}

std::string ConditionallyIdentifiedContactTransfer::generate_passwords()const
{
    return contact_transfer_request_generate_passwords();
}

std::string IdentifiedContactTransfer::get_public_request_type()const
{
    static const std::string type = "mojeid_identified_contact_transfer";
    return type;
}

std::string IdentifiedContactTransfer::generate_passwords()const
{
    return contact_transfer_request_generate_passwords();
}

}//Fred::MojeID::PublicRequest
}//Fred::MojeID
}//Fred
