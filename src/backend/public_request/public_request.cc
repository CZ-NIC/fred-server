#include "src/backend/public_request/public_request.hh"

#include "src/backend/buffer.hh"
#include "src/backend/public_request/exceptions.hh"
#include "src/backend/public_request/object_type.hh"
#include "src/backend/public_request/type/public_request_authinfo.hh"
#include "src/backend/public_request/type/public_request_blockunblock.hh"
#include "src/backend/public_request/type/public_request_personal_info.hh"
#include "src/backend/public_request/type/get_iface_of.hh"
#include "src/libfred/object/get_id_of_registered.hh"
#include "src/libfred/object/object_states_info.hh"
#include "src/libfred/public_request/create_public_request.hh"
#include "src/libfred/public_request/info_public_request.hh"
#include "src/libfred/public_request/public_request_lock_guard.hh"
#include "src/libfred/public_request/public_request_status.hh"
#include "src/libfred/public_request/public_request_type_iface.hh"
#include "src/libfred/public_request/update_public_request.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "src/util/types/stringify.hh"
#include "src/util/random.hh"
#include "src/util/log/context.hh"

#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>

#include <sstream>
#include <stdexcept>
#include <utility>
#include <map>

namespace Fred {
namespace Backend {
namespace PublicRequest {

namespace {

std::map<std::string, unsigned char> get_public_request_type_to_post_type_dictionary()
{
    std::map<std::string, unsigned char> dictionary;
    if (dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::AuthinfoPost>()
                               .get_public_request_type(), 1)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::BlockTransfer<PublicRequestImpl::ConfirmedBy::letter>>()
                               .get_public_request_type(), 2)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::UnblockTransfer<PublicRequestImpl::ConfirmedBy::letter>>()
                               .get_public_request_type(), 3)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::BlockChanges<PublicRequestImpl::ConfirmedBy::letter>>()
                               .get_public_request_type(), 4)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::UnblockChanges<PublicRequestImpl::ConfirmedBy::letter>>()
                               .get_public_request_type(), 5)).second &&
        dictionary.insert(
                std::make_pair(Type::get_iface_of<Type::PersonalInfoPost>()
                               .get_public_request_type(), 6)).second)
    {
        return dictionary;
    }
    throw std::logic_error("duplicate public request type");
}

short public_request_type_to_post_type(const std::string& public_request_type)
{
    typedef std::map<std::string, unsigned char> Dictionary;
    static const Dictionary dictionary = get_public_request_type_to_post_type_dictionary();
    const Dictionary::const_iterator result_ptr = dictionary.find(public_request_type);
    const bool key_found = result_ptr != dictionary.end();
    if (key_found)
    {
        return result_ptr->second;
    }
    throw InvalidPublicRequestType();
}

std::string language_to_lang_code(PublicRequestImpl::Language::Enum lang)
{
    switch (lang)
    {
        case PublicRequestImpl::Language::cs:
            return "cs";
        case PublicRequestImpl::Language::en:
            return "en";
    }
    throw std::invalid_argument("language code not found");
}

std::string create_ctx_name(const std::string& _name)
{
    return str(boost::format("%1%-<%2%>") % _name % Random::integer(0, 10000));
}

std::string create_ctx_function_name(const char* fnc)
{
    std::string name(fnc);
    std::replace(name.begin(), name.end(), '_', '-');
    return name;
}

class LogContext
{
public:
    LogContext(const PublicRequestImpl& _impl, const std::string& _op_name)
        : ctx_server_(create_ctx_name(_impl.get_server_name())),
          ctx_interface_("PublicRequest"),
          ctx_operation_(_op_name)
    {
    }

private:
    Logging::Context ctx_server_;
    Logging::Context ctx_interface_;
    Logging::Context ctx_operation_;
};

} // namespace Fred::Backend::PublicRequest::{anonymous}

#define LOGGING_CONTEXT(CTX_VAR, IMPL_OBJ) \
    LogContext CTX_VAR((IMPL_OBJ), create_ctx_function_name(__FUNCTION__))

PublicRequestImpl::PublicRequestImpl(const std::string& _server_name)
    : server_name_(_server_name)
{
    LogContext log_ctx(*this, "init");
}

PublicRequestImpl::~PublicRequestImpl()
{
}

const std::string& PublicRequestImpl::get_server_name() const
{
    return server_name_;
}

unsigned long long PublicRequestImpl::create_authinfo_request_registry_email(
        ObjectType::Enum object_type,
        const std::string& object_handle,
        const Optional<unsigned long long>& log_request_id) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        const auto object_id = get_id_of_registered_object(ctx, object_type, object_handle);
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(object_id).exec(ctx));
        LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, object_id);
        if (states.presents(LibFred::Object_State::server_transfer_prohibited))
        {
            throw ObjectTransferProhibited();
        }
        const auto public_request_id = LibFred::CreatePublicRequest()
            .exec(locked_object, Type::get_iface_of<Type::AuthinfoAuto>(), log_request_id);
        LibFred::UpdatePublicRequest()
            .set_status(LibFred::PublicRequest::Status::resolved)
            .exec(locked_object, Type::get_iface_of<Type::AuthinfoAuto>(), log_request_id);
        ctx.commit_transaction();

        return public_request_id;
    }
    catch (const NoPublicRequest& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectNotFound();
    }
    catch (const NoContactEmail& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw NoContactEmail();
    }
    catch (const LibFred::UnknownObject& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectNotFound();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("create_authinfo_request (registry) failed due to an unknown exception");
        throw;
    }
}

unsigned long long PublicRequestImpl::create_authinfo_request_non_registry_email(
        ObjectType::Enum object_type,
        const std::string& object_handle,
        const Optional<unsigned long long>& log_request_id,
        ConfirmedBy::Enum confirmation_method,
        const std::string& specified_email) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        const auto object_id = get_id_of_registered_object(ctx, object_type, object_handle);
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(object_id).exec(ctx));
        LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, object_id);
        if (states.presents(LibFred::Object_State::server_transfer_prohibited))
        {
            throw ObjectTransferProhibited();
        }
        const auto create_public_request_op = LibFred::CreatePublicRequest().set_email_to_answer(specified_email);
        switch (confirmation_method)
        {
            case ConfirmedBy::email:
            {
                const unsigned long long request_id =
                    create_public_request_op.exec(locked_object, Type::get_iface_of<Type::AuthinfoEmail>(), log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case ConfirmedBy::letter:
            {
                const unsigned long long request_id =
                    create_public_request_op.exec(locked_object, Type::get_iface_of<Type::AuthinfoPost>(), log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
        }
        throw std::runtime_error("unexpected confirmation method");
    }
    catch (const LibFred::UnknownObject& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectNotFound();
    }
    catch (const LibFred::CreatePublicRequest::Exception& e)
    {
        if (e.is_set_wrong_email())
        {
            LOGGER(PACKAGE).info(boost::diagnostic_information(e));
            throw InvalidContactEmail();
        }
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("create_authinfo_request (non registry) failed due to an unknown exception");
        throw;
    }
}

unsigned long long PublicRequestImpl::create_block_unblock_request(
        ObjectType::Enum object_type,
        const std::string& object_handle,
        const Optional<unsigned long long>& log_request_id,
        ConfirmedBy::Enum confirmation_method,
        LockRequestType::Enum lock_request_type) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        const unsigned long long object_id = get_id_of_registered_object(ctx, object_type, object_handle);
        LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, object_id);
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(object_id).exec(ctx));
        if (states.presents(LibFred::Object_State::mojeid_contact) ||
            states.presents(LibFred::Object_State::server_blocked))
        {
            throw OperationProhibited();
        }
        switch (lock_request_type)
        {
            case LockRequestType::block_transfer:
            {
                if (states.presents(LibFred::Object_State::server_transfer_prohibited))
                {
                    throw ObjectAlreadyBlocked();
                }
                const unsigned long long request_id =
                        LibFred::CreatePublicRequest().exec(
                                locked_object,
                                Type::get_iface_of<Type::BlockTransfer>(confirmation_method),
                                log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case LockRequestType::block_transfer_and_update:
            {
                if (states.presents(LibFred::Object_State::server_transfer_prohibited) &&
                    states.presents(LibFred::Object_State::server_update_prohibited))
                {
                    throw ObjectAlreadyBlocked();
                }
                const unsigned long long request_id =
                        LibFred::CreatePublicRequest().exec(
                                locked_object,
                                Type::get_iface_of<Type::BlockChanges>(confirmation_method),
                                log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case LockRequestType::unblock_transfer:
            {
                if (states.presents(LibFred::Object_State::server_update_prohibited))
                {
                    throw HasDifferentBlock();
                }
                if (states.absents(LibFred::Object_State::server_transfer_prohibited))
                {
                    throw ObjectNotBlocked();
                }
                const unsigned long long request_id =
                        LibFred::CreatePublicRequest().exec(
                                locked_object,
                                Type::get_iface_of<Type::UnblockTransfer>(confirmation_method),
                                log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case LockRequestType::unblock_transfer_and_update:
            {
                if (states.absents(LibFred::Object_State::server_update_prohibited))
                {
                    if (states.presents(LibFred::Object_State::server_transfer_prohibited))
                    {
                        throw HasDifferentBlock();
                    }
                    throw ObjectNotBlocked();
                }
                const unsigned long long request_id =
                        LibFred::CreatePublicRequest().exec(
                                locked_object,
                                Type::get_iface_of<Type::UnblockChanges>(confirmation_method),
                                log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
        }
        throw std::runtime_error("unexpected lock request type");
    }
    catch (const LibFred::UnknownObject& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectNotFound();
    }
    catch (const OperationProhibited& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw OperationProhibited();
    }
    catch (const ObjectAlreadyBlocked& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectAlreadyBlocked();
    }
    catch (const ObjectNotBlocked& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectNotBlocked();
    }
    catch (const HasDifferentBlock& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw HasDifferentBlock();
    }
    catch (const LibFred::CreatePublicRequest::Exception& e)
    {
        if (e.is_set_wrong_email())
        {
            LOGGER(PACKAGE).info(boost::diagnostic_information(e));
            throw InvalidContactEmail();
        }
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("Unknown error");
        throw;
    }
}

unsigned long long PublicRequestImpl::create_personal_info_request_registry_email(
        const std::string& contact_handle,
        const Optional<unsigned long long>& log_request_id) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        const auto contact_id = LibFred::get_id_of_registered<LibFred::Object_Type::contact>(ctx, contact_handle);
        LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, contact_id);
        const auto public_request_id = LibFred::CreatePublicRequest()
            .exec(locked_object, Type::get_iface_of<Type::PersonalInfoAuto>(), log_request_id);
        LibFred::UpdatePublicRequest()
            .set_status(LibFred::PublicRequest::Status::resolved)
            .exec(locked_object, Type::get_iface_of<Type::PersonalInfoAuto>(), log_request_id);
        ctx.commit_transaction();

        return public_request_id;
    }
    catch (const NoPublicRequest& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectNotFound();
    }
    catch (const NoContactEmail& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw NoContactEmail();
    }
    catch (const LibFred::UnknownObject& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectNotFound();
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("create_personal_info_request (registry) failed due to an unknown exception");
        throw;
    }
}

unsigned long long PublicRequestImpl::create_personal_info_request_non_registry_email(
        const std::string& contact_handle,
        const Optional<unsigned long long>& log_request_id,
        ConfirmedBy::Enum confirmation_method,
        const std::string& specified_email) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        const auto contact_id = LibFred::get_id_of_registered<LibFred::Object_Type::contact>(ctx, contact_handle);
        LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, contact_id);
        const auto create_public_request_op = LibFred::CreatePublicRequest().set_email_to_answer(specified_email);
        switch (confirmation_method)
        {
            case ConfirmedBy::email:
            {
                const unsigned long long request_id =
                    create_public_request_op.exec(locked_object, Type::get_iface_of<Type::PersonalInfoEmail>(), log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case ConfirmedBy::letter:
            {
                const unsigned long long request_id =
                    create_public_request_op.exec(locked_object, Type::get_iface_of<Type::PersonalInfoPost>(), log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
        }
        throw std::runtime_error("unexpected confirmation method");
    }
    catch (const LibFred::UnknownObject& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectNotFound();
    }
    catch (const LibFred::CreatePublicRequest::Exception& e)
    {
        if (e.is_set_wrong_email())
        {
            LOGGER(PACKAGE).info(boost::diagnostic_information(e));
            throw InvalidContactEmail();
        }
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).error("create_personal_info_request (non registry) failed due to an unknown exception");
        throw;
    }
}

Fred::Backend::Buffer PublicRequestImpl::create_public_request_pdf(
        unsigned long long public_request_id,
        Language::Enum lang,
        std::shared_ptr<LibFred::Document::Manager> manager) const
{
    LOGGING_CONTEXT(log_ctx, *this);
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

std::shared_ptr<LibFred::Document::Manager> PublicRequestImpl::get_default_document_manager()
{
    const HandleRegistryArgs* const args = CfgArgs::instance()->get_handler_ptr_by_type<HandleRegistryArgs>();
    return std::shared_ptr<LibFred::Document::Manager>(
            LibFred::Document::Manager::create(
                    args->docgen_path,
                    args->docgen_template_path,
                    args->fileclient_path,
                    CorbaContainer::get_instance()->getNS()->getHostName()));
}

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
