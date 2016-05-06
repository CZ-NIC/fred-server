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

class ContactConditionalIdentificationFake:public PublicRequestTypeIface
{
public:
    ~ContactConditionalIdentificationFake() { }
    std::string get_public_request_type()const { return "contact_conditional_identification"; }
    const PublicRequestTypeIface& iface()const { return *this; }
private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create()const
    {
        throw std::runtime_error("get_public_request_types_to_cancel_on_create method should never be called");
    }
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
        Fred::PublicRequest::Status::Enum _old_status, Fred::PublicRequest::Status::Enum _new_status)const
    {
        if ((_old_status == Fred::PublicRequest::Status::active) &&
            (_new_status == Fred::PublicRequest::Status::invalidated)) {
            return PublicRequestTypes();
        }
        throw std::runtime_error("get_public_request_types_to_cancel_on_update method can be used "
                                 "for invalidating of active requests only");
    }
};

class ContactIdentificationFake:public PublicRequestTypeIface
{
public:
    ~ContactIdentificationFake() { }
    std::string get_public_request_type()const { return "contact_identification"; }
    const PublicRequestTypeIface& iface()const { return *this; }
private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create()const
    {
        throw std::runtime_error("get_public_request_types_to_cancel_on_create method should never be called");
    }
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
        Fred::PublicRequest::Status::Enum _old_status, Fred::PublicRequest::Status::Enum _new_status)const
    {
        if ((_old_status == Fred::PublicRequest::Status::active) &&
            (_new_status == Fred::PublicRequest::Status::invalidated)) {
            return PublicRequestTypes();
        }
        throw std::runtime_error("get_public_request_types_to_cancel_on_update method can be used "
                                 "for invalidating of active requests only");
    }
};

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

PublicRequestTypeIface::PublicRequestTypes
ContactConditionalIdentification::get_public_request_types_to_cancel_on_create()const
{
    PublicRequestTypes result;
    result.insert(IfacePtr(new ContactConditionalIdentification));
    return result;
}

PublicRequestTypeIface::PublicRequestTypes
ContactConditionalIdentification::get_public_request_types_to_cancel_on_update(
    Fred::PublicRequest::Status::Enum _old_status, Fred::PublicRequest::Status::Enum _new_status)const
{
    PublicRequestTypes result;
    if ((_old_status == Fred::PublicRequest::Status::active) &&
        (_new_status == Fred::PublicRequest::Status::answered)) {
        result.insert(IfacePtr(new Fred::ContactConditionalIdentificationFake));
    }
    return result;
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

PublicRequestTypeIface::PublicRequestTypes
ContactIdentification::get_public_request_types_to_cancel_on_create()const
{
    PublicRequestTypes result;
    result.insert(IfacePtr(new ContactIdentification));
    result.insert(IfacePtr(new Fred::ContactIdentificationFake));
    return result;
}

PublicRequestTypeIface::PublicRequestTypes
ContactIdentification::get_public_request_types_to_cancel_on_update(
    Fred::PublicRequest::Status::Enum _old_status, Fred::PublicRequest::Status::Enum _new_status)const
{
    PublicRequestTypes result;
    if ((_old_status == Fred::PublicRequest::Status::active) &&
        (_new_status == Fred::PublicRequest::Status::answered)) {
    }
    return result;
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

PublicRequestTypeIface::PublicRequestTypes
ContactReidentification::get_public_request_types_to_cancel_on_create()const
{
    PublicRequestTypes result;
    result.insert(IfacePtr(new ContactReidentification));
    return result;
}

PublicRequestTypeIface::PublicRequestTypes
ContactReidentification::get_public_request_types_to_cancel_on_update(
    Fred::PublicRequest::Status::Enum _old_status, Fred::PublicRequest::Status::Enum _new_status)const
{
    PublicRequestTypes result;
    if ((_old_status == Fred::PublicRequest::Status::active) &&
        (_new_status == Fred::PublicRequest::Status::answered)) {
    }
    return result;
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

PublicRequestTypeIface::PublicRequestTypes
ContactValidation::get_public_request_types_to_cancel_on_create()const
{
    PublicRequestTypes result;
    result.insert(IfacePtr(new ContactValidation));
    return result;
}

PublicRequestTypeIface::PublicRequestTypes
ContactValidation::get_public_request_types_to_cancel_on_update(
    Fred::PublicRequest::Status::Enum _old_status, Fred::PublicRequest::Status::Enum _new_status)const
{
    PublicRequestTypes result;
    if ((_old_status == Fred::PublicRequest::Status::active) &&
        (_new_status == Fred::PublicRequest::Status::answered)) {
    }
    return result;
}

std::string ConditionallyIdentifiedContactTransfer::get_public_request_type()const
{
    return "mojeid_conditionally_identified_contact_transfer";
}

PublicRequestTypeIface::PublicRequestTypes
ConditionallyIdentifiedContactTransfer::get_public_request_types_to_cancel_on_create()const
{
    PublicRequestTypes result;
    result.insert(IfacePtr(new ConditionallyIdentifiedContactTransfer));
    return result;
}

PublicRequestTypeIface::PublicRequestTypes
ConditionallyIdentifiedContactTransfer::get_public_request_types_to_cancel_on_update(
    Fred::PublicRequest::Status::Enum _old_status, Fred::PublicRequest::Status::Enum _new_status)const
{
    PublicRequestTypes result;
    if ((_old_status == Fred::PublicRequest::Status::active) &&
        (_new_status == Fred::PublicRequest::Status::answered)) {
        result.insert(IfacePtr(new Fred::ContactIdentificationFake));
        result.insert(IfacePtr(new PrevalidatedContactTransfer));
    }
    return result;
}

std::string ConditionallyIdentifiedContactTransfer::generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const
{
    return contact_transfer_request_generate_passwords(_locked_contact);
}

std::string IdentifiedContactTransfer::get_public_request_type()const
{
    return "mojeid_identified_contact_transfer";
}

PublicRequestTypeIface::PublicRequestTypes
IdentifiedContactTransfer::get_public_request_types_to_cancel_on_create()const
{
    PublicRequestTypes result;
    result.insert(IfacePtr(new IdentifiedContactTransfer));
    return result;
}

PublicRequestTypeIface::PublicRequestTypes
IdentifiedContactTransfer::get_public_request_types_to_cancel_on_update(
    Fred::PublicRequest::Status::Enum _old_status, Fred::PublicRequest::Status::Enum _new_status)const
{
    PublicRequestTypes result;
    if ((_old_status == Fred::PublicRequest::Status::active) &&
        (_new_status == Fred::PublicRequest::Status::answered)) {
    }
    return result;
}

std::string IdentifiedContactTransfer::generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const
{
    return contact_transfer_request_generate_passwords(_locked_contact);
}

std::string PrevalidatedUnidentifiedContactTransfer::get_public_request_type()const
{
    return "mojeid_prevalidated_unidentified_contact_transfer";
}

PublicRequestTypeIface::PublicRequestTypes
PrevalidatedUnidentifiedContactTransfer::get_public_request_types_to_cancel_on_create()const
{
    PublicRequestTypes result;
    result.insert(IfacePtr(new PrevalidatedUnidentifiedContactTransfer));
    result.insert(IfacePtr(new Fred::ContactConditionalIdentificationFake));
    result.insert(IfacePtr(new ContactConditionalIdentification));
    return result;
}

PublicRequestTypeIface::PublicRequestTypes
PrevalidatedUnidentifiedContactTransfer::get_public_request_types_to_cancel_on_update(
    Fred::PublicRequest::Status::Enum _old_status, Fred::PublicRequest::Status::Enum _new_status)const
{
    PublicRequestTypes result;
    if ((_old_status == Fred::PublicRequest::Status::active) &&
        (_new_status == Fred::PublicRequest::Status::answered)) {
    }
    return result;
}

std::string PrevalidatedUnidentifiedContactTransfer::generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const
{
    return ContactConditionalIdentification().iface().generate_passwords(_locked_contact);
}

std::string PrevalidatedContactTransfer::get_public_request_type()const
{
    return "mojeid_prevalidated_contact_transfer";
}

PublicRequestTypeIface::PublicRequestTypes
PrevalidatedContactTransfer::get_public_request_types_to_cancel_on_create()const
{
    PublicRequestTypes result;
    result.insert(IfacePtr(new PrevalidatedContactTransfer));
    result.insert(IfacePtr(new ConditionallyIdentifiedContactTransfer));
    result.insert(IfacePtr(new IdentifiedContactTransfer));
    return result;
}

PublicRequestTypeIface::PublicRequestTypes
PrevalidatedContactTransfer::get_public_request_types_to_cancel_on_update(
    Fred::PublicRequest::Status::Enum _old_status, Fred::PublicRequest::Status::Enum _new_status)const
{
    PublicRequestTypes result;
    if ((_old_status == Fred::PublicRequest::Status::active) &&
        (_new_status == Fred::PublicRequest::Status::answered)) {
    }
    return result;
}

std::string PrevalidatedContactTransfer::generate_passwords(const LockedPublicRequestsOfObjectForUpdate &_locked_contact)const
{
    return contact_transfer_request_generate_passwords(_locked_contact);
}

}//Fred::MojeID::PublicRequest
}//Fred::MojeID
}//Fred
