#include "src/backend/public_request/process_public_requests.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrar/info_registrar.hh"
#include "src/backend/public_request/send_email.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "src/libfred/public_request/public_request_lock_guard.hh"
#include "src/libfred/public_request/info_public_request.hh"
#include "src/deprecated/model/public_request_filter.hh"
#include "src/backend/public_request/public_request.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {

namespace {

void set_on_status_action(
        unsigned long long _public_request_id,
        LibFred::PublicRequest::OnStatusAction::Enum _action,
        LibFred::OperationContext& _ctx)
{
    const Database::Result dbres =
        _ctx.get_conn().exec_params("UPDATE public_request "
                                    "SET on_status_action=$1::enum_on_status_action_type "
                                    "WHERE id=$2::BIGINT;",
                                    Database::query_param_list
                                    (Conversion::Enums::to_db_handle(_action))
                                    (_public_request_id));

    if (dbres.rows_affected() == 1)
    {
        return;
    }
    throw std::runtime_error("failed to mark a public_request as processed");
}

template<typename T>
std::string pretty_print_address(const T& _address)
{
    std::ostringstream address;
    address << _address.street1 << ", ";
    address << _address.city << ", ";
    address << _address.postalcode;
    return address.str();
}

unsigned long long send_personalinfo(
        unsigned long long _public_request_id,
        LibFred::OperationContext& _ctx)
{
    const std::shared_ptr<LibFred::Mailer::Manager> manager = std::make_shared<MailerManager>(CorbaContainer::get_instance()->getNS()); // sigh
    LibFred::PublicRequestLockGuardById locked_request(_ctx, _public_request_id);
    const LibFred::PublicRequestInfo request_info = LibFred::InfoPublicRequest().exec(_ctx, locked_request);
    const auto contact_id = request_info.get_object_id().get_value(); // oops
    LibFred::Mailer::Parameters email_template_params;

    const Database::Result dbres = _ctx.get_conn().exec_params(
            "SELECT (create_time AT TIME ZONE 'UTC' AT TIME ZONE 'Europe/Prague')::DATE FROM public_request "
            "WHERE id=$1::BIGINT",
            Database::query_param_list(_public_request_id));
    if (dbres.size() < 1)
    {
        throw NoPublicRequest();
    }
    if (1 < dbres.size())
    {
        throw std::runtime_error("too many public requests for given id");
    }

    LibFred::InfoContactData info_contact_data;
    try
    {
        info_contact_data = LibFred::InfoContactById(contact_id).exec(_ctx).info_contact_data;
    }
    catch (const LibFred::InfoContactByHandle::Exception& ex)
    {
        if (ex.is_set_unknown_contact_handle())
        {
            throw PublicRequestImpl::NoContactEmail();
        }
        throw;
    }
    email_template_params.insert(LibFred::Mailer::Parameters::value_type("handle", info_contact_data.handle));
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("organization", info_contact_data.organization.get_value_or_default()));
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("name", info_contact_data.name.get_value_or_default()));
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("address",
                                                    info_contact_data.place.isnull()
                                                    ? std::string()
                                                    : pretty_print_address(info_contact_data.place.get_value())));
    std::string mailing_address;
    std::string billing_address;
    std::string shipping_address_1;
    std::string shipping_address_2;
    std::string shipping_address_3;

    for (const auto& address : info_contact_data.addresses)
    {
        switch (address.first.value)
        {
            case LibFred::ContactAddressType::MAILING:
                mailing_address = pretty_print_address(address.second);
                break;
            case LibFred::ContactAddressType::BILLING:
                billing_address = pretty_print_address(address.second);
                break;
            case LibFred::ContactAddressType::SHIPPING:
                shipping_address_1 = pretty_print_address(address.second);
                break;
            case LibFred::ContactAddressType::SHIPPING_2:
                shipping_address_2 = pretty_print_address(address.second);
                break;
            case LibFred::ContactAddressType::SHIPPING_3:
                shipping_address_3 = pretty_print_address(address.second);
                break;
        }
    }
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("mailing_address", mailing_address));
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("billing_address", billing_address));
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("shipping_address_1", shipping_address_1));
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("shipping_address_2", shipping_address_2));
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("shipping_address_3", shipping_address_3));
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("ident_type", info_contact_data.ssntype.get_value_or_default()));
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("ident_value", info_contact_data.ssn.get_value_or_default()));
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("dic", info_contact_data.vat.get_value_or_default()));
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("telephone", info_contact_data.telephone.get_value_or_default()));
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("fax", info_contact_data.fax.get_value_or_default()));
    if (!info_contact_data.email.isnull())
    {
        throw PublicRequestImpl::NoContactEmail();
    }
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("email", info_contact_data.email.get_value()));
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("notify_email", info_contact_data.notifyemail.get_value_or_default()));

    LibFred::InfoRegistrarData info_registrar_data;
    try
    {
        info_registrar_data = LibFred::InfoRegistrarByHandle(info_contact_data.sponsoring_registrar_handle)
            .exec(_ctx).info_registrar_data;
        email_template_params.insert(
                LibFred::Mailer::Parameters::value_type("registrar_name", info_registrar_data.name.get_value_or_default()));
        email_template_params.insert(
                LibFred::Mailer::Parameters::value_type("registrar_url", info_registrar_data.url.get_value_or_default()));
    }
    catch (const LibFred::InfoRegistrarByHandle::Exception& ex)
    {
        email_template_params.insert(
                LibFred::Mailer::Parameters::value_type("registrar_name", std::string()));
        email_template_params.insert(
                LibFred::Mailer::Parameters::value_type("registrar_url", std::string()));
    }

    // create attachment
    std::vector<unsigned long long> attachments;
    // add attachments number
    const std::set<std::string> recipients = { info_contact_data.email.get_value() };
    const EmailData data(recipients, "sendpersonalinfo_pif", email_template_params, attachments);
    return send_joined_addresses_email(manager, data);
}

} // namespace Fred::Backend::PublicRequest::{anonymous}

void process_public_request_nop(
        unsigned long long _public_request_id,
        LibFred::OperationContext& _ctx)
{
    set_on_status_action(_public_request_id, LibFred::PublicRequest::OnStatusAction::processed, _ctx);
}

void process_public_request_personal_info_answered(
        unsigned long long _public_request_id,
        LibFred::OperationContext& _ctx)
{
    try
    {
        send_personalinfo(_public_request_id, _ctx);
    }
    catch (...)
    {
        set_on_status_action(_public_request_id, LibFred::PublicRequest::OnStatusAction::failed, _ctx);
        throw;
    }
    set_on_status_action(_public_request_id, LibFred::PublicRequest::OnStatusAction::processed, _ctx);
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
