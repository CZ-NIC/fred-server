#include "src/backend/public_request/process_public_requests.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"
#include "src/libfred/registrable_object/contact/info_contact.hh"
#include "src/libfred/registrar/info_registrar.hh"
#include "src/backend/public_request/send_email.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "src/libfred/public_request/public_request_lock_guard.hh"
#include "src/libfred/public_request/info_public_request.hh"
#include "src/libfred/public_request/update_public_request.hh"
#include "src/backend/public_request/public_request.hh"
#include "src/libfred/opcontext.hh"
#include "src/util/csv/csv.hh"

#include <array>

namespace Fred {
namespace Backend {
namespace PublicRequest {

namespace {

template<typename T>
std::string pretty_print_address(const T& _address)
{
    std::ostringstream address;
    address << _address.street1 << ", ";
    if (!_address.street2.get_value_or_default().empty())
    {
        address << _address.street2.get_value() << ", ";
    }
    if (!_address.street3.get_value_or_default().empty())
    {
        address << _address.street3.get_value() << ", ";
    }
    address << _address.city << ", ";
    if (!_address.stateorprovince.get_value_or_default().empty())
    {
        address << _address.stateorprovince.get_value() << ", ";
    }
    address << _address.country << ", ";
    address << _address.postalcode;
    return address.str();
}

unsigned long long send_personalinfo(
        unsigned long long _public_request_id,
        LibFred::OperationContext& _ctx,
        std::shared_ptr<LibFred::Mailer::Manager> _mailer_manager,
        std::shared_ptr<LibFred::File::Transferer> _file_manager_client)
{
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
    const std::string email_to_answer = request_info.get_email_to_answer().get_value_or_default();

    LibFred::InfoContactData info_contact_data;
    try
    {
        info_contact_data = LibFred::InfoContactById(contact_id).exec(_ctx).info_contact_data;
    }
    catch (const LibFred::InfoContactByHandle::Exception& ex)
    {
        if (ex.is_set_unknown_contact_handle())
        {
            throw NoContactEmail();
        }
        throw;
    }
    email_template_params.insert(LibFred::Mailer::Parameters::value_type("handle", info_contact_data.handle));
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("organization", info_contact_data.organization.get_value_or_default()));
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("name", info_contact_data.name.get_value_or_default()));
    const std::string pretty_printed_address = info_contact_data.place.isnull()
        ? std::string()
        : pretty_print_address(info_contact_data.place.get_value());
    email_template_params.insert(
            LibFred::Mailer::Parameters::value_type("address", pretty_printed_address));
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
    if (info_contact_data.email.isnull())
    {
        throw NoContactEmail();
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


    const std::string ident_type = info_contact_data.ssntype.get_value_or_default();

    std::vector<unsigned long long> attachments;
    attachments.reserve(2);
    constexpr unsigned db_enum_filetype_dot_personal_info_csv = 12;
    constexpr char separator = ';';
    {
        std::string ident_type_repr;
        if (ident_type == "RC")
        {
            ident_type_repr = "Rodné číslo";
        }
        else if (ident_type == "OP")
        {
            ident_type_repr = "Číslo OP";
        }
        else if (ident_type == "PASS")
        {
            ident_type_repr = "Číslo pasu";
        }
        else if (ident_type == "ICO")
        {
            ident_type_repr = "IČO";
        }
        else if (ident_type == "MPSV")
        {
            ident_type_repr = "Identifikátor MPSV";
        }
        else if (ident_type == "BIRTHDAY")
        {
            ident_type_repr = "Datum narození";
        }
        const std::string csv_document_content =
            Fred::Util::to_csv_string_using_separator<separator>(std::vector<std::array<std::string, 2>>({
                        {"ID kontaktu v registru", info_contact_data.handle},
                        {"Organizace", info_contact_data.organization.get_value_or_default()},
                        {"Jméno", info_contact_data.name.get_value_or_default()},
                        {"Adresa trvalého bydliště", pretty_printed_address},
                        {"Korespondenční adresa", mailing_address},
                        {"Fakturační adresa", billing_address},
                        {"Dodací adresa 1", shipping_address_1},
                        {"Dodací adresa 2", shipping_address_2},
                        {"Dodací adresa 3", shipping_address_3},
                        {"Typ identifikace", ident_type_repr},
                        {"Identifikace", info_contact_data.ssn.get_value_or_default()},
                        {"DIČ", info_contact_data.vat.get_value_or_default()},
                        {"Telefon", info_contact_data.telephone.get_value_or_default()},
                        {"Fax", info_contact_data.fax.get_value_or_default()},
                        {"E-mail", info_contact_data.email.get_value()},
                        {"Notifikační e-mail", info_contact_data.notifyemail.get_value_or_default()},
                        {"Určený registrátor", info_registrar_data.name.get_value_or_default()}
                    }));


        std::vector<char> in_buffer(csv_document_content.begin(), csv_document_content.end());
        const unsigned long long attachment_id = _file_manager_client->upload(
                in_buffer,
                "personal_info_cs.csv",
                "text/csv",
                db_enum_filetype_dot_personal_info_csv);
        attachments.emplace_back(attachment_id);
    }

    {
        std::string ident_type_repr;
        if (ident_type == "RC")
        {
            ident_type_repr = "National Identity Number";
        }
        else if (ident_type == "OP")
        {
            ident_type_repr = "National Identity Card";
        }
        else if (ident_type == "PASS")
        {
            ident_type_repr = "Passport Number";
        }
        else if (ident_type == "ICO")
        {
            ident_type_repr = "Company Registration Number";
        }
        else if (ident_type == "MPSV")
        {
            ident_type_repr = "Social Security Number";
        }
        else if (ident_type == "BIRTHDAY")
        {
            ident_type_repr = "Birthdate";
        }

        const std::string csv_document_content =
            Fred::Util::to_csv_string_using_separator<separator>(std::vector<std::array<std::string, 2>>({
                        {"Contact ID in the registry", info_contact_data.handle},
                        {"Organisation", info_contact_data.organization.get_value_or_default()},
                        {"Name", info_contact_data.name.get_value_or_default()},
                        {"Address", pretty_printed_address},
                        {"Mailing address", mailing_address},
                        {"Billing address", billing_address},
                        {"Shipping address 1", shipping_address_1},
                        {"Shipping address 2", shipping_address_2},
                        {"Shipping address 3", shipping_address_3},
                        {"Identification type", ident_type_repr},
                        {"Identification", info_contact_data.ssn.get_value_or_default()},
                        {"VAT No.", info_contact_data.vat.get_value_or_default()},
                        {"Phone", info_contact_data.telephone.get_value_or_default()},
                        {"Fax", info_contact_data.fax.get_value_or_default()},
                        {"E-mail", info_contact_data.email.get_value()},
                        {"Notification e-mail", info_contact_data.notifyemail.get_value_or_default()},
                        {"Designated registrar", info_registrar_data.name.get_value_or_default()}
                    }));

        std::vector<char> in_buffer(csv_document_content.begin(), csv_document_content.end());
        const unsigned long long attachment_id = _file_manager_client->upload(
                in_buffer,
                "personal_info_en.csv",
                "text/csv",
                db_enum_filetype_dot_personal_info_csv);
        attachments.emplace_back(attachment_id);
    }

    const std::set<std::string> recipients = { email_to_answer.empty() ? info_contact_data.email.get_value() : email_to_answer };
    const EmailData data(recipients, "sendpersonalinfo_pif", email_template_params, attachments);
    return send_joined_addresses_email(_mailer_manager, data);
}

} // namespace Fred::Backend::PublicRequest::{anonymous}

void process_public_request_personal_info_resolved(
        unsigned long long _public_request_id,
        const LibFred::PublicRequestTypeIface& _public_request_type,
        std::shared_ptr<LibFred::Mailer::Manager> _mailer_manager,
        std::shared_ptr<LibFred::File::Transferer> _file_manager_client)
{
    try
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::PublicRequestLockGuardById locked_request(ctx, _public_request_id);
        const unsigned long long email_id = send_personalinfo(_public_request_id, ctx, _mailer_manager, _file_manager_client);
        try
        {
            LibFred::UpdatePublicRequest()
                .set_answer_email_id(email_id)
                .set_on_status_action(LibFred::PublicRequest::OnStatusAction::processed)
                .exec(locked_request, _public_request_type);
            ctx.commit_transaction();
        }
        catch (const std::exception& e)
        {
            ctx.get_log().info(
                    boost::format("Request %1% update failed (%2%), but email %3% sent") %
                    _public_request_id %
                    e.what() %
                    email_id);
        }
        catch (...)
        {
            ctx.get_log().info(
                    boost::format("Request %1% update failed (unknown exception), but email %2% sent") %
                    _public_request_id %
                    email_id);
        }
    }
    catch (...)
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::PublicRequestLockGuardById locked_request(ctx, _public_request_id);
        LibFred::UpdatePublicRequest()
            .set_on_status_action(LibFred::PublicRequest::OnStatusAction::failed)
            .exec(locked_request, _public_request_type);
        ctx.commit_transaction();
        throw;
    }
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
