#include "src/mojeid/mojeid_public_request.h"
#include "util/random.h"
#include "util/cfg/handle_contactverification_args.h"
#include "util/cfg/handle_mojeid_args.h"
#include "util/cfg/config_handler_decl.h"

namespace Fred {

namespace Password {

const ::size_t chunk_length = 8;

std::string generate(::size_t _length = chunk_length)
{
    const std::string set_of_possible_values = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789";
    return Random::string_from(_length, set_of_possible_values);
}

}//Fred::Password

namespace {

std::string get_demo_pin1()//11111111
{
    const std::string pin1(Password::chunk_length, '1');
    return pin1;
}

std::string get_demo_pin2()//22222222
{
    const std::string pin2(Password::chunk_length, '2');
    return pin2;
}

std::string get_demo_pin3()//33333333
{
    const std::string pin3(Password::chunk_length, '3');
    return pin3;
}

std::string get_demo_pin1_pin2()
{
    const std::string pin1_pin2 = get_demo_pin1() +
                                  get_demo_pin2();
    return pin1_pin2;
}

std::string generate_pin()
{
    return Password::generate();
}

std::string generate_pin1()
{
    return Password::generate();
}

std::string generate_pin2()
{
    return generate_pin();
}

std::string generate_pin3()
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
    const bool runs_in_demo_mode =
        CfgArgs::instance()->get_handler_ptr_by_type< HandleContactVerificationArgs >()->demo_mode;

    return runs_in_demo_mode ? get_demo_pin1_pin2()
                             : generate_pin1_pin2();
}

namespace MojeID {

std::string contact_transfer_request_generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)
{
    const bool runs_in_demo_mode =
        CfgArgs::instance()->get_handler_ptr_by_type< HandleMojeIDArgs >()->demo_mode;

    if (runs_in_demo_mode) {
        return get_demo_pin1();
    } 

    const Database::Result res = _locked_contact.get_ctx().get_conn().exec_params(
        "WITH object_authinfopw AS "
            "(SELECT SUBSTRING(COALESCE(authinfopw,'') FOR $2::INTEGER) AS passwd "
             "FROM object "
             "WHERE id=$1::BIGINT) "
        "SELECT passwd,LENGTH(passwd) "
        "FROM object_authinfopw",
        Database::query_param_list(_locked_contact.get_id())//$1::BIGINT
                                  (Password::chunk_length));//$2::INTEGER
    if (res.size() <= 0) {
        throw std::runtime_error("object not found");
    }
    const std::string authinfopw = static_cast< std::string >(res[0][0]);
    const ::size_t authinfopw_length = static_cast< ::size_t >(res[0][1]);
    if (Password::chunk_length <= authinfopw_length) {
        return authinfopw;
    }
    return authinfopw + Password::generate(Password::chunk_length - authinfopw_length);
}

std::string contact_identification_generate_passwords()
{
    const bool runs_in_demo_mode =
        CfgArgs::instance()->get_handler_ptr_by_type< HandleMojeIDArgs >()->demo_mode;

    return runs_in_demo_mode ? get_demo_pin3()
                             : generate_pin3();
}

namespace PublicRequest {

std::string ContactConditionalIdentification::get_pin1_part(const std::string &_summary_password)
{
    //first part is utf-8 encoded so its length is variable
    //length of second part is always Password::chunk_length
    return _summary_password.substr(0, _summary_password.length() - Password::chunk_length);
}

std::string ContactConditionalIdentification::get_pin2_part(const std::string &_summary_password)
{
    //first part is utf-8 encoded so its length is variable
    //length of second part is always Password::chunk_length
    return _summary_password.substr(_summary_password.length() - Password::chunk_length, Password::chunk_length);
}

std::string ContactConditionalIdentification::get_public_request_type()const
{
    return "mojeid_contact_conditional_identification";
}

std::string ContactConditionalIdentification::generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const
{
    const std::string cci_pass = conditional_contact_identification_generate_passwords();
    const std::string mtr_pass = contact_transfer_request_generate_passwords(_locked_contact);
    /* merge transfer pin with cond. contact identification */
    return mtr_pass + get_pin2_part(cci_pass);
}

std::string ContactIdentification::get_public_request_type()const
{
    return "mojeid_contact_identification";
}

std::string ContactIdentification::generate_passwords(const LockedPublicRequestsOfObjectForUpdate&)const
{
    const std::string ci_pass = contact_identification_generate_passwords();
    return ci_pass;
}

std::string ContactReidentification::get_public_request_type()const
{
    return "mojeid_contact_reidentification";
}

std::string ContactReidentification::generate_passwords(const LockedPublicRequestsOfObjectForUpdate&)const
{
    const std::string ci_pass = contact_identification_generate_passwords();
    return ci_pass;
}

std::string ContactValidation::get_public_request_type()const
{
    return "mojeid_contact_validation";
}

std::string ConditionallyIdentifiedContactTransfer::get_public_request_type()const
{
    return "mojeid_conditionally_identified_contact_transfer";
}

std::string ConditionallyIdentifiedContactTransfer::generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const
{
    return contact_transfer_request_generate_passwords(_locked_contact);
}

std::string IdentifiedContactTransfer::get_public_request_type()const
{
    return "mojeid_identified_contact_transfer";
}

std::string IdentifiedContactTransfer::generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const
{
    return contact_transfer_request_generate_passwords(_locked_contact);
}

std::string PrevalidatedUnidentifiedContactTransfer::get_public_request_type()const
{
    return "mojeid_prevalidated_unidentified_contact_transfer";
}

std::string PrevalidatedUnidentifiedContactTransfer::generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const
{
    return ContactConditionalIdentification().iface().generate_passwords(_locked_contact);
}

std::string PrevalidatedContactTransfer::get_public_request_type()const
{
    return "mojeid_prevalidated_contact_transfer";
}

std::string PrevalidatedContactTransfer::generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const
{
    return contact_transfer_request_generate_passwords(_locked_contact);
}

}//Fred::MojeID::PublicRequest
}//Fred::MojeID
}//Fred
