/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/backend/public_request/create_public_request_pdf.hh"

#include "src/backend/public_request/confirmed_by.hh"
#include "src/backend/public_request/exceptions.hh"
#include "src/backend/public_request/language.hh"
#include "src/backend/public_request/type/get_iface_of.hh"
#include "src/backend/public_request/type/public_request_authinfo.hh"
#include "src/backend/public_request/type/public_request_block_unblock.hh"
#include "src/backend/public_request/type/public_request_personal_info.hh"
#include "src/libfred/object/object_states_info.hh"
#include "src/libfred/opcontext.hh"
#include "src/libfred/public_request/create_public_request.hh"
#include "src/libfred/public_request/info_public_request.hh"
#include "src/libfred/public_request/public_request_lock_guard.hh"
#include "src/libfred/public_request/update_public_request.hh"
#include "src/util/log/context.hh"
#include "src/util/optional_value.hh"
#include "src/util/types/stringify.hh"

namespace Fred {
namespace Backend {
namespace PublicRequest {

namespace {

std::map<std::string, unsigned char> get_public_request_type_to_post_or_government_type_dictionary()
{
    std::map<std::string, unsigned char> dictionary;
    if (dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::AuthinfoPost>()
                               .get_public_request_type(), 1)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::AuthinfoGovernment>()
                               .get_public_request_type(), 1)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::BlockTransfer<ConfirmedBy::letter>>()
                               .get_public_request_type(), 2)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::BlockTransfer<ConfirmedBy::government>>()
                               .get_public_request_type(), 2)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::UnblockTransfer<ConfirmedBy::letter>>()
                               .get_public_request_type(), 3)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::UnblockTransfer<ConfirmedBy::government>>()
                               .get_public_request_type(), 3)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::BlockChanges<ConfirmedBy::letter>>()
                               .get_public_request_type(), 4)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::BlockChanges<ConfirmedBy::government>>()
                               .get_public_request_type(), 4)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::UnblockChanges<ConfirmedBy::letter>>()
                               .get_public_request_type(), 5)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::UnblockChanges<ConfirmedBy::government>>()
                               .get_public_request_type(), 5)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::PersonalInfoPost>()
                               .get_public_request_type(), 6)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::PersonalInfoGovernment>()
                               .get_public_request_type(), 8)).second)
    {
        return dictionary;
    }
    throw std::logic_error("duplicate public request type");
}

short public_request_type_to_post_type(const std::string& public_request_type)
{
    typedef std::map<std::string, unsigned char> Dictionary;
    static const Dictionary dictionary = get_public_request_type_to_post_or_government_type_dictionary();
    const Dictionary::const_iterator result_ptr = dictionary.find(public_request_type);
    const bool key_found = result_ptr != dictionary.end();
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
    std::string create_time;
    std::string email_to_answer;
    unsigned long long post_type;
    try
    {
        LibFred::PublicRequestLockGuardById locked_request(ctx, public_request_id);
        const LibFred::PublicRequestInfo request_info = LibFred::InfoPublicRequest().exec(ctx, locked_request);
        post_type = public_request_type_to_post_type(request_info.get_type());
        create_time = stringify(request_info.get_create_time().date());
        email_to_answer = request_info.get_email_to_answer().get_value_or_default();
    }
    catch (const LibFred::PublicRequestLockGuardById::Exception&)
    {
        throw ObjectNotFound();
    }

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
    std::ostringstream pdf_content;
    const std::unique_ptr<LibFred::Document::Generator> docgen_ptr(
            manager.get()->createOutputGenerator(
                    LibFred::Document::GT_PUBLIC_REQUEST_PDF,
                    pdf_content,
                    lang_code));
    docgen_ptr->getInput()
            // clang-format off
            << "<?xml version='1.0' encoding='utf-8'?>"
            << "<enum_whois>"
            << "<public_request>"
                << "<type>" << post_type << "</type>"
                << "<handle type='" << type_id << "'>"
                << handle
                << "</handle>"
                << "<date>" << create_time << "</date>"
                << "<id>" << public_request_id << "</id>"
                << "<replymail>" << email_to_answer << "</replymail>"
            << "</public_request>"
            << "</enum_whois>";
            // clang-format on
    docgen_ptr->closeInput();

    return Fred::Backend::Buffer(pdf_content.str());
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
