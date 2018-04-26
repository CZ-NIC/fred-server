#include "src/libfred/public_request/create_public_request.hh"
#include "src/libfred/public_request/update_public_request.hh"
#include "src/libfred/public_request/public_request_lock_guard.hh"
#include "src/libfred/public_request/public_request_status.hh"
#include "src/libfred/contact_verification/django_email_format.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"
#include "src/util/idn_utils.hh"

#include <string>

namespace LibFred {

CreatePublicRequest::CreatePublicRequest(const Optional< std::string > &_reason,
                                         const Optional< std::string > &_email_to_answer,
                                         const Optional< RegistrarId > &_registrar_id)
:   reason_(_reason),
    email_to_answer_(_email_to_answer),
    registrar_id_(_registrar_id)
{
}

CreatePublicRequest& CreatePublicRequest::set_reason(const std::string &_reason)
{
    reason_ = _reason;
    return *this;
}

CreatePublicRequest& CreatePublicRequest::set_email_to_answer(const std::string &_email)
{
    email_to_answer_ = _email;
    return *this;
}

CreatePublicRequest& CreatePublicRequest::set_registrar_id(RegistrarId _id)
{
    registrar_id_ = _id;
    return *this;
}

namespace {

void check_email_address_format(const std::string& email_address)
{
    const unsigned max_length_of_email_address = 255;
    if (max_length_of_email_address < Util::get_utf8_char_len(email_address))
    {
        BOOST_THROW_EXCEPTION(CreatePublicRequest::Exception().set_wrong_email(email_address));
    }
    const bool email_address_format_is_valid = DjangoEmailFormat().check(email_address);
    if (!email_address_format_is_valid)
    {
        BOOST_THROW_EXCEPTION(CreatePublicRequest::Exception().set_wrong_email(email_address));
    }
}

} // namespace LibFred::{anonymous}

PublicRequestId CreatePublicRequest::exec(const LockedPublicRequestsOfObjectForUpdate &_locked_object,
                                          const PublicRequestTypeIface &_type,
                                          const Optional< LogRequestId > &_create_log_request_id)const
{
    try {
        cancel_on_create(_type, _locked_object, registrar_id_, _create_log_request_id);
        const std::string public_request_type = _type.get_public_request_type();
        Database::query_param_list params(public_request_type);                             // $1::TEXT
        params(_locked_object.get_id())                                                     // $2::BIGINT
              (reason_.isset() ? reason_.get_value() : Database::QPNull);                   // $3::TEXT
        if (email_to_answer_.isset())
        {
            const std::string email_address = email_to_answer_.get_value();
            check_email_address_format(email_address);
            params(email_address);                                                          // $4::TEXT
        }
        else {
            params(Database::QPNull);                                                       // $4::TEXT
        }
        if (registrar_id_.isset()) {
            const RegistrarId registrar_id = registrar_id_.get_value();
            const bool registrar_id_exists = static_cast< bool >(_locked_object.get_ctx().get_conn().exec_params(
                "SELECT EXISTS(SELECT * FROM registrar WHERE id=$1::BIGINT)",
                Database::query_param_list(registrar_id))[0][0]);
            if (!registrar_id_exists) {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_registrar_id(registrar_id));
            }
            params(registrar_id);                                                           // $5::BIGINT
        }
        else {
            params(Database::QPNull);                                                       // $5::BIGINT
        }
        if (_create_log_request_id.isset()) {
            params(_create_log_request_id.get_value());                                     // $6::BIGINT
        }
        else {
            params(Database::QPNull);                                                       // $6::BIGINT
        }
        params(Conversion::Enums::to_db_handle(PublicRequest::Status::active));             // $7::TEXT

        const auto on_status_action = _type.get_on_status_action(PublicRequest::Status::active);
        params(Conversion::Enums::to_db_handle(on_status_action)); // $8::ENUM_ON_STATUS_ACTION_TYPE

        const Database::Result res = _locked_object.get_ctx().get_conn().exec_params(
            "WITH request AS ("
                "INSERT INTO public_request "
                    "(request_type,status,resolve_time,reason,email_to_answer,answer_email_id,registrar_id,"
                     "create_request_id,resolve_request_id,on_status_action) "
                "SELECT eprt.id,eprs.id,NULL,$3::TEXT,$4::TEXT,NULL,$5::BIGINT,$6::BIGINT,NULL,$8::ENUM_ON_STATUS_ACTION_TYPE "
                "FROM enum_public_request_type eprt,"
                     "enum_public_request_status eprs "
                "WHERE eprt.name=$1::TEXT AND "
                      "eprs.name=$7::TEXT "
                "RETURNING id) "
            "INSERT INTO public_request_objects_map (request_id,object_id) "
                "SELECT id,$2::BIGINT FROM request "
            "RETURNING request_id", params);
        if (0 < res.size()) {
            const PublicRequestId public_request_id = static_cast< PublicRequestId >(res[0][0]);
            return public_request_id;
        }
        BOOST_THROW_EXCEPTION(Exception().set_unknown_type(public_request_type));
    }
    catch (const Exception&) {
        throw;
    }
    catch (const std::runtime_error&) {
        throw;
    }
}

::size_t CreatePublicRequest::cancel_on_create(const PublicRequestTypeIface &_type_to_create,
                                               const LockedPublicRequestsOfObjectForUpdate &_locked_object,
                                               const Optional< RegistrarId > _registrar_id,
                                               const Optional< LogRequestId > &_log_request_id)
{
    const PublicRequestTypeIface::PublicRequestTypes to_cancel =
        _type_to_create.get_public_request_types_to_cancel_on_create();
    ::size_t number_of_cancelled = 0;
    for (PublicRequestTypeIface::PublicRequestTypes::const_iterator to_cancel_ptr = to_cancel.begin();
         to_cancel_ptr != to_cancel.end(); ++to_cancel_ptr)
    {
        Database::query_param_list params(_locked_object.get_id());            // $1::BIGINT
        params((*to_cancel_ptr)->get_public_request_type());                   // $2::TEXT
        params(Conversion::Enums::to_db_handle(PublicRequest::Status::active));// $3::TEXT
        const Database::Result res = _locked_object.get_ctx().get_conn().exec_params(
            "SELECT pr.id "
            "FROM public_request pr "
            "JOIN public_request_objects_map prom ON prom.request_id=pr.id "
            "WHERE prom.object_id=$1::BIGINT AND "
                  "pr.request_type=(SELECT id FROM enum_public_request_type WHERE name=$2::TEXT) AND "
                  "pr.status=(SELECT id FROM enum_public_request_status WHERE name=$3::TEXT)", params);
        for (::size_t idx = 0; idx < res.size(); ++idx) {
            const PublicRequestId public_request_id = static_cast< PublicRequestId >(res[idx][0]);
            UpdatePublicRequest update_public_request_op;
            update_public_request_op.set_status(PublicRequest::Status::invalidated);
            if (_registrar_id.isset()) {
                update_public_request_op.set_registrar_id(_registrar_id.get_value());
            }
            PublicRequestLockGuardById locked_public_request(_locked_object.get_ctx(), public_request_id);
            update_public_request_op.exec(locked_public_request, **to_cancel_ptr, _log_request_id);
        }
        number_of_cancelled += res.size();
    }
    return number_of_cancelled;
}

} // namespace LibFred
