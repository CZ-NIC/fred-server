/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/public_request/create_public_request_pdf.hh"

#include "src/backend/public_request/confirmed_by.hh"
#include "src/backend/public_request/exceptions.hh"
#include "src/backend/public_request/language.hh"
#include "src/backend/public_request/type/get_iface_of.hh"
#include "src/backend/public_request/type/public_request_authinfo.hh"
#include "src/backend/public_request/type/public_request_block_unblock.hh"
#include "src/backend/public_request/type/public_request_personal_info.hh"
#include "libfred/object/object_states_info.hh"
#include "libfred/opcontext.hh"
#include "libfred/public_request/create_public_request.hh"
#include "libfred/public_request/info_public_request.hh"
#include "libfred/public_request/public_request_lock_guard.hh"
#include "libfred/public_request/update_public_request.hh"
#include "util/log/context.hh"
#include "util/optional_value.hh"
#include "src/util/types/stringify.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {

namespace {

struct TemplateParams
{
    TemplateParams(unsigned int _type, const std::string& _confirmation_type) :
        type(_type), confirmation_type(_confirmation_type)
    {}

    unsigned int type;
    std::string confirmation_type;
};


std::map<std::string, TemplateParams> get_public_request_type_to_template_params_mapping()
{
    const std::string confirmation_type_notarized_letter = "notarized_letter";
    const std::string confirmation_type_government = "government";

    std::map<std::string, TemplateParams> dictionary;
    if (dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::AuthinfoPost>()
                               .get_public_request_type(), TemplateParams(1, confirmation_type_notarized_letter))).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::AuthinfoGovernment>()
                               .get_public_request_type(), TemplateParams(1, confirmation_type_government))).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::BlockTransfer<ConfirmedBy::letter>>()
                               .get_public_request_type(), TemplateParams(2, confirmation_type_notarized_letter))).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::BlockTransfer<ConfirmedBy::government>>()
                               .get_public_request_type(), TemplateParams(2, confirmation_type_government))).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::UnblockTransfer<ConfirmedBy::letter>>()
                               .get_public_request_type(), TemplateParams(3, confirmation_type_notarized_letter))).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::UnblockTransfer<ConfirmedBy::government>>()
                               .get_public_request_type(), TemplateParams(3, confirmation_type_government))).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::BlockChanges<ConfirmedBy::letter>>()
                               .get_public_request_type(), TemplateParams(4, confirmation_type_notarized_letter))).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::BlockChanges<ConfirmedBy::government>>()
                               .get_public_request_type(), TemplateParams(4, confirmation_type_government))).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::UnblockChanges<ConfirmedBy::letter>>()
                               .get_public_request_type(), TemplateParams(5, confirmation_type_notarized_letter))).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::UnblockChanges<ConfirmedBy::government>>()
                               .get_public_request_type(), TemplateParams(5, confirmation_type_government))).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::PersonalInfoPost>()
                               .get_public_request_type(), TemplateParams(6, confirmation_type_notarized_letter))).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::PersonalInfoGovernment>()
                               .get_public_request_type(), TemplateParams(6, confirmation_type_government))).second)
    {
        return dictionary;
    }
    throw std::logic_error("duplicate public request type");
}

TemplateParams public_request_type_to_template_params(const std::string& public_request_type)
{
    typedef std::map<std::string, TemplateParams> TemplateParamsMapping;
    static const TemplateParamsMapping mapping = get_public_request_type_to_template_params_mapping();
    const TemplateParamsMapping::const_iterator result_ptr = mapping.find(public_request_type);
    const bool key_found = result_ptr != mapping.end();
    if (key_found)
    {
        return result_ptr->second;
    }
    throw InvalidPublicRequestType();
}

std::string language_to_lang_code(Language::Enum lang)
{
    switch (lang)
    {
        case Language::cs:
            return "cs";
        case Language::en:
            return "en";
    }
    throw std::invalid_argument("language code not found");
}

std::string create_ctx_function_name(const char *fnc)
{
    std::string name(fnc);
    std::replace(name.begin(), name.end(), '_', '-');
    return name;
}

class LogContext
{
public:
    LogContext(const std::string &_op_name)
        : ctx_operation_(_op_name)
    { }
private:
    Logging::Context ctx_operation_;
};

#define LOGGING_CONTEXT(CTX_VAR) LogContext CTX_VAR(create_ctx_function_name(__FUNCTION__))

} // namespace Fred::Backend::PublicRequest::{anonymous}

Fred::Backend::Buffer create_public_request_pdf(
        const unsigned long long public_request_id,
        Language::Enum lang,
        std::shared_ptr<LibFred::Document::Manager> manager)
{
    LOGGING_CONTEXT(log_ctx);
    const std::string lang_code = language_to_lang_code(lang);

    LibFred::OperationContextCreator ctx;

    const LibFred::PublicRequestInfo request_info = [&ctx, public_request_id]() {
        try {
            LibFred::PublicRequestLockGuardById locked_request(ctx, public_request_id);
            return LibFred::InfoPublicRequest().exec(ctx, locked_request);
        }
        catch (const LibFred::PublicRequestLockGuardById::Exception&)
        {
            throw ObjectNotFound();
        }
    }();

    const Database::Result dbres = ctx.get_conn().exec_params(
            "SELECT oreg.type,oreg.name "
            "FROM public_request pr "
            "JOIN public_request_objects_map prom ON prom.request_id=pr.id "
            "JOIN object_registry oreg ON oreg.id=prom.object_id "
            "WHERE pr.id=$1::BIGINT",
            Database::query_param_list(public_request_id));
    if (dbres.size() != 1)
    {
        if (dbres.size() == 0)
        {
            throw ObjectNotFound();
        }
        throw std::runtime_error("too many objects associated with this public request");
    }
    const unsigned type_id = static_cast<unsigned>(dbres[0][0]);
    const std::string handle = static_cast<std::string>(dbres[0][1]);
    const TemplateParams params = public_request_type_to_template_params(request_info.get_type());

    std::ostringstream pdf_content;
    const std::unique_ptr<LibFred::Document::Generator> docgen_ptr(
            manager.get()->createOutputGenerator(
                    LibFred::Document::GT_PUBLIC_REQUEST_PDF,
                    pdf_content,
                    lang_code));
    std::ostringstream xml_content;
    // clang-format off
    xml_content << "<?xml version='1.0' encoding='utf-8'?>"
                << "<enum_whois>"
                << "<public_request>"
                    << "<type>" << params.type << "</type>"
                    << "<confirmation_type>" << params.confirmation_type << "</confirmation_type>"
                    << "<handle type='" << type_id << "'>"
                    << handle
                    << "</handle>"
                    << "<date>" << stringify(request_info.get_create_time().date()) << "</date>"
                    << "<id>" << public_request_id << "</id>"
                    << "<replymail>" << request_info.get_email_to_answer().get_value_or_default() << "</replymail>"
                << "</public_request>"
                << "</enum_whois>";
    // clang-format on
    ctx.get_log().debug(xml_content.str());
    docgen_ptr->getInput() << xml_content.str();
    docgen_ptr->closeInput();

    return Fred::Backend::Buffer(pdf_content.str());
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
