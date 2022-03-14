/*
 * Copyright (C) 2018-2022  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/backend/public_request/process/personal_info.hh"

#include "src/backend/public_request/exceptions.hh"
#include "src/backend/public_request/util/send_joined_address_email.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "src/util/csv/csv.hh"

#include "libfiled/libfiled.hh"
#include "libfred/opcontext.hh"
#include "libfred/public_request/info_public_request.hh"
#include "libfred/public_request/public_request_lock_guard.hh"
#include "libfred/public_request/public_request_on_status_action.hh"
#include "libfred/public_request/update_public_request.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrar/info_registrar.hh"

#include <array>
#include <sstream>

namespace Fred {
namespace Backend {
namespace PublicRequest {
namespace Process {

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

unsigned long long send_personal_info(
        const LibFred::LockedPublicRequestForUpdate& _locked_request,
        const MessengerArgs& _messenger_args,
        const FilemanArgs& _fileman_args)
{
    auto& ctx = _locked_request.get_ctx();
    const LibFred::PublicRequestInfo request_info = LibFred::InfoPublicRequest().exec(ctx, _locked_request);
    const auto contact_id = request_info.get_object_id().get_value(); // oops

    const std::string email_to_answer = request_info.get_email_to_answer().get_value_or_default();

    LibFred::InfoContactData info_contact_data;
    try
    {
        info_contact_data = LibFred::InfoContactById(contact_id).exec(ctx).info_contact_data;
    }
    catch (const LibFred::InfoContactByHandle::Exception& ex)
    {
        if (ex.is_set_unknown_contact_handle())
        {
            throw NoContactEmail();
        }
        throw;
    }

    LibFred::Mailer::Parameters email_template_params;

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
            .exec(ctx).info_registrar_data;
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

    LibFiled::Connection<LibFiled::Service::File> connection{
        LibFiled::Connection<LibFiled::Service::File>::ConnectionString{
            _fileman_endpoint}};

    const std::string ident_type = info_contact_data.ssntype.get_value_or_default();
    constexpr char separator = ';';

    const auto get_attachment_cs = [&](){
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
        std::istringstream csv_document_stream(csv_document_content);

        const auto attachment_uuid =
                LibFiled::File::create(
                        connection,
                        LibFiled::File::FileName{"personal_info_cs.csv"},
                        csv_document_stream,
                        LibFiled::File::FileMimeType{"text/csv"});

        return *attachment_uuid;
    };

    const auto get_attachment_en = [&](){
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
        std::istringstream csv_document_stream(csv_document_content);

        const auto attachment_uuid =
                LibFiled::File::create(
                        connection,
                        LibFiled::File::FileName{"personal_info_en.csv"},
                        csv_document_stream,
                        LibFiled::File::FileMimeType{"text/csv"});

        return *attachment_uuid;
    };

    const std::set<std::string> recipients = { email_to_answer.empty() ? info_contact_data.email.get_value() : email_to_answer };
    const Util::EmailData email_data(
            recipients,
            "send-personalinfo-pif-subject.txt",
            "send-personalinfo-pif-body.txt",
            email_template_params,
            {get_attachment_cs(),
             get_attachment_en()});

    return send_joined_addresses_email(_messenger_args.endpoint, _messenger_args.archive, email_data);
}

} // namespace Fred::Backend::PublicRequest::Process::{anonymous}

void process_public_request_personal_info_resolved(
        unsigned long long _public_request_id,
        const LibFred::PublicRequestTypeIface& _public_request_type,
        const MessengerArgs& _messenger_args,
        const FilemanArgs& _fileman_args)
{
    try
    {
        LibFred::OperationContextCreator ctx;
        const LibFred::PublicRequestLockGuardById locked_request(ctx, _public_request_id);
        const unsigned long long email_id = send_personal_info(locked_request, _messenger_args, _fileman_args);
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

} // namespace Fred::Backend::PublicRequest::Process
} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
