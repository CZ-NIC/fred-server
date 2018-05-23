#include "src/backend/public_request/public_request.hh"

#include "src/backend/buffer.hh"
#include "src/libfred/object/get_id_of_registered.hh"
#include "src/libfred/object/object_state.hh"
#include "src/libfred/object/object_states_info.hh"
#include "src/libfred/object_state/get_object_states.hh"
#include "src/libfred/public_request/create_public_request.hh"
#include "src/libfred/public_request/info_public_request.hh"
#include "src/libfred/public_request/public_request_lock_guard.hh"
#include "src/libfred/public_request/public_request_status.hh"
#include "src/libfred/public_request/public_request_on_status_action.hh"
#include "src/libfred/public_request/public_request_type_iface.hh"
#include "src/libfred/public_request/update_public_request.hh"
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/corba_wrapper_decl.hh"
#include "src/util/db/query_param.hh"
#include "src/util/log/context.hh"
#include "src/util/random.hh"
#include "src/util/types/stringify.hh"
#include "src/backend/public_request/send_email.hh"
#include "src/backend/public_request/get_types.hh"

#include <boost/exception/diagnostic_information.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace Fred {
namespace Backend {
namespace PublicRequest {

namespace Type {

namespace {

template <class T>
struct ImplementedBy
{
    template <const char* name>
    class Named : public LibFred::PublicRequestTypeIface
    {
    public:
        typedef T Type;
        Named()
            : implementation_()
        {
        }
        std::string get_public_request_type() const
        {
            return name;
        }

    private:
        PublicRequestTypes get_public_request_types_to_cancel_on_create() const
        {
            return implementation_.template get_public_request_types_to_cancel_on_create<Named>();
        }
        PublicRequestTypes get_public_request_types_to_cancel_on_update(
                LibFred::PublicRequest::Status::Enum _old_status,
                LibFred::PublicRequest::Status::Enum _new_status) const
        {
            return implementation_.template get_public_request_types_to_cancel_on_update<Named>(
                    _old_status,
                    _new_status);
        }
        LibFred::PublicRequest::OnStatusAction::Enum get_on_status_action(
                LibFred::PublicRequest::Status::Enum _status) const
        {
            return implementation_.template get_on_status_action<Named>(_status);
        }
        const Type implementation_;
    };
};


struct AuthinfoImplementation
{
    template <typename T>
    LibFred::PublicRequestTypeIface::PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        LibFred::PublicRequestTypeIface::PublicRequestTypes res;
        res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new T));
        return res;
    }
    template <typename T>
    LibFred::PublicRequestTypeIface::PublicRequestTypes get_public_request_types_to_cancel_on_update(
            LibFred::PublicRequest::Status::Enum,
            LibFred::PublicRequest::Status::Enum) const
    {
        return LibFred::PublicRequestTypeIface::PublicRequestTypes();
    };
    template <typename T>
    LibFred::PublicRequest::OnStatusAction::Enum get_on_status_action(LibFred::PublicRequest::Status::Enum _status) const
    {
        return LibFred::PublicRequest::OnStatusAction::processed;
    };
};

typedef ImplementedBy<AuthinfoImplementation> AuthinfoPublicRequest;

extern const char authinfo_auto_pif[] = "authinfo_auto_pif";
typedef AuthinfoPublicRequest::Named<authinfo_auto_pif> AuthinfoAuto;

extern const char authinfo_email_pif[] = "authinfo_email_pif";
typedef AuthinfoPublicRequest::Named<authinfo_email_pif> AuthinfoEmail;

extern const char authinfo_post_pif[] = "authinfo_post_pif";
typedef AuthinfoPublicRequest::Named<authinfo_post_pif> AuthinfoPost;


struct PersonalinfoImplementation
{
    template <typename T>
    LibFred::PublicRequestTypeIface::PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        return LibFred::PublicRequestTypeIface::PublicRequestTypes();
    }
    template <typename T>
    LibFred::PublicRequestTypeIface::PublicRequestTypes get_public_request_types_to_cancel_on_update(
            LibFred::PublicRequest::Status::Enum,
            LibFred::PublicRequest::Status::Enum) const
    {
        return LibFred::PublicRequestTypeIface::PublicRequestTypes();
    };

    template <typename T>
    LibFred::PublicRequest::OnStatusAction::Enum get_on_status_action(LibFred::PublicRequest::Status::Enum _status) const
    {
        if (_status == LibFred::PublicRequest::Status::resolved)
        {
            return LibFred::PublicRequest::OnStatusAction::scheduled;
        }
        return LibFred::PublicRequest::OnStatusAction::processed;
    };
};

typedef ImplementedBy<PersonalinfoImplementation> PersonalinfoPublicRequest;

extern const char personalinfo_auto_pif[] = "personalinfo_auto_pif";
typedef PersonalinfoPublicRequest::Named<personalinfo_auto_pif> PersonalinfoAuto;

extern const char personalinfo_email_pif[] = "personalinfo_email_pif";
typedef PersonalinfoPublicRequest::Named<personalinfo_email_pif> PersonalinfoEmail;

extern const char personalinfo_post_pif[] = "personalinfo_post_pif";
typedef AuthinfoPublicRequest::Named<personalinfo_post_pif> PersonalinfoPost;


LibFred::PublicRequestTypeIface::PublicRequestTypes get_block_unblock_public_request_types_to_cancel_on_create();

struct BlockUnblockImplementation
{
    template <typename>
    LibFred::PublicRequestTypeIface::PublicRequestTypes get_public_request_types_to_cancel_on_create() const
    {
        return get_block_unblock_public_request_types_to_cancel_on_create();
    }
    template <typename T>
    LibFred::PublicRequestTypeIface::PublicRequestTypes get_public_request_types_to_cancel_on_update(
            LibFred::PublicRequest::Status::Enum,
            LibFred::PublicRequest::Status::Enum) const
    {
        return LibFred::PublicRequestTypeIface::PublicRequestTypes();
    };
    template <typename T>
    LibFred::PublicRequest::OnStatusAction::Enum get_on_status_action(LibFred::PublicRequest::Status::Enum _status) const
    {
        return LibFred::PublicRequest::OnStatusAction::processed;
    };
};

extern const char block_changes_email_pif[] = "block_changes_email_pif";
extern const char block_changes_post_pif[] = "block_changes_post_pif";

extern const char block_transfer_email_pif[] = "block_transfer_email_pif";
extern const char block_transfer_post_pif[] = "block_transfer_post_pif";

extern const char unblock_changes_email_pif[] = "unblock_changes_email_pif";
extern const char unblock_changes_post_pif[] = "unblock_changes_post_pif";

extern const char unblock_transfer_email_pif[] = "unblock_transfer_email_pif";
extern const char unblock_transfer_post_pif[] = "unblock_transfer_post_pif";

struct Block
{
    typedef ImplementedBy<BlockUnblockImplementation> PublicRequest;
    struct Changes
    {
        typedef PublicRequest::Named<block_changes_email_pif> ByEmail;
        typedef PublicRequest::Named<block_changes_post_pif> ByPost;
    };

    struct Transfer
    {
        typedef PublicRequest::Named<block_transfer_email_pif> ByEmail;
        typedef PublicRequest::Named<block_transfer_post_pif> ByPost;
    };
};

struct Unblock
{
    typedef ImplementedBy<BlockUnblockImplementation> PublicRequest;
    struct Changes
    {
        typedef PublicRequest::Named<unblock_changes_email_pif> ByEmail;
        typedef PublicRequest::Named<unblock_changes_post_pif> ByPost;
    };

    struct Transfer
    {
        typedef PublicRequest::Named<unblock_transfer_email_pif> ByEmail;
        typedef PublicRequest::Named<unblock_transfer_post_pif> ByPost;
    };
};

LibFred::PublicRequestTypeIface::PublicRequestTypes get_block_unblock_public_request_types_to_cancel_on_create()
{
    LibFred::PublicRequestTypeIface::PublicRequestTypes res;
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Block::Changes::ByEmail));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Block::Changes::ByPost));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Block::Transfer::ByEmail));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Block::Transfer::ByPost));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Unblock::Changes::ByEmail));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Unblock::Changes::ByPost));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Unblock::Transfer::ByEmail));
    res.insert(LibFred::PublicRequestTypeIface::IfacePtr(new Unblock::Transfer::ByPost));
    return res;
}

} // namespace Fred::Backend::PublicRequest::Type::{anonymous}

} // namespace Fred::Backend::PublicRequest::Type

namespace {

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

#define LOGGING_CONTEXT(CTX_VAR, IMPL_OBJ) LogContext CTX_VAR((IMPL_OBJ), create_ctx_function_name(__FUNCTION__))

unsigned long long send_authinfo(
        unsigned long long public_request_id,
        const std::string& handle,
        PublicRequestImpl::ObjectType::Enum object_type,
        std::shared_ptr<LibFred::Mailer::Manager> manager)
{
    LibFred::OperationContextCreator ctx;
    LibFred::Mailer::Parameters email_template_params;
    {
        const Database::Result dbres = ctx.get_conn().exec_params(
                "SELECT (create_time AT TIME ZONE 'UTC' AT TIME ZONE 'Europe/Prague')::DATE FROM public_request "
                "WHERE id=$1::BIGINT",
                Database::query_param_list(public_request_id));
        if (dbres.size() < 1)
        {
            throw NoPublicRequest();
        }
        if (1 < dbres.size())
        {
            throw std::runtime_error("too many public requests for given id");
        }
        email_template_params.insert(LibFred::Mailer::Parameters::value_type("reqid", boost::lexical_cast<std::string>(public_request_id)));
        email_template_params.insert(LibFred::Mailer::Parameters::value_type("reqdate", static_cast<std::string>(dbres[0][0])));
        email_template_params.insert(LibFred::Mailer::Parameters::value_type("handle", handle));
    }

    std::string sql;
    std::string object_type_handle;
    switch (object_type)
    {
        case PublicRequestImpl::ObjectType::contact:
            // clang-format off
            sql = "SELECT o.authinfopw,TRIM(c.email) "
                  "FROM object o "
                  "JOIN object_registry obr ON obr.id=o.id "
                  "JOIN contact c ON c.id=o.id "
                  "WHERE obr.name=UPPER($1::TEXT) AND "
                        "obr.type=get_object_type_id($2::TEXT) AND "
                        "obr.erdate IS NULL AND "
                        "COALESCE(TRIM(c.email),'')<>''";
            // clang-format on
            object_type_handle = Conversion::Enums::to_db_handle(LibFred::Object_Type::contact);
            email_template_params.insert(LibFred::Mailer::Parameters::value_type("type", "1"));
            break;
        case PublicRequestImpl::ObjectType::nsset:
            // clang-format off
            sql = "SELECT o.authinfopw,TRIM(c.email) "
                  "FROM object o "
                  "JOIN object_registry obr ON obr.id=o.id "
                  "JOIN nsset n ON n.id=o.id "
                  "JOIN nsset_contact_map ncm ON ncm.nssetid=n.id "
                  "JOIN contact c ON c.id=ncm.contactid "
                  "WHERE obr.name=UPPER($1::TEXT) AND "
                        "obr.type=get_object_type_id($2::TEXT) AND "
                        "obr.erdate IS NULL AND "
                        "COALESCE(TRIM(c.email),'')<>''";
            // clang-format on
            object_type_handle = Conversion::Enums::to_db_handle(LibFred::Object_Type::nsset);
            email_template_params.insert(LibFred::Mailer::Parameters::value_type("type", "2"));
            break;
        case PublicRequestImpl::ObjectType::domain:
            // clang-format off
            sql = "SELECT o.authinfopw,TRIM(c.email) "
                  "FROM object o "
                  "JOIN object_registry obr ON obr.id=o.id "
                  "JOIN domain d ON d.id=o.id "
                  "JOIN contact c ON c.id=d.registrant "
                  "WHERE obr.name=LOWER($1::TEXT) AND "
                        "obr.type=get_object_type_id($2::TEXT) AND "
                        "obr.erdate IS NULL AND "
                        "COALESCE(TRIM(c.email),'')<>'' "
              "UNION "
                  "SELECT o.authinfopw,TRIM(c.email) "
                  "FROM object o "
                  "JOIN object_registry obr ON obr.id=o.id "
                  "JOIN domain d ON d.id=o.id "
                  "JOIN domain_contact_map dcm ON dcm.domainid=d.id "
                  "JOIN contact c ON c.id=dcm.contactid AND c.id!=d.registrant "
                  "WHERE obr.name=LOWER($1::TEXT) AND "
                        "obr.type=get_object_type_id($2::TEXT) AND "
                        "obr.erdate IS NULL AND "
                        "COALESCE(TRIM(c.email),'')<>'' AND "
                        "dcm.role=1";
            // clang-format on
            object_type_handle = Conversion::Enums::to_db_handle(LibFred::Object_Type::domain);
            email_template_params.insert(LibFred::Mailer::Parameters::value_type("type", "3"));
            break;
        case PublicRequestImpl::ObjectType::keyset:
            // clang-format off
            sql = "SELECT o.authinfopw,TRIM(c.email) "
                  "FROM object o "
                  "JOIN object_registry obr ON obr.id=o.id "
                  "JOIN keyset k ON k.id=o.id "
                  "JOIN keyset_contact_map kcm ON kcm.keysetid=k.id "
                  "JOIN contact c ON c.id=kcm.contactid "
                  "WHERE obr.name=UPPER($1::TEXT) AND "
                        "obr.type=get_object_type_id($2::TEXT) AND "
                        "obr.erdate IS NULL AND "
                        "COALESCE(TRIM(c.email),'')<>''";
            // clang-format on
            object_type_handle = Conversion::Enums::to_db_handle(LibFred::Object_Type::keyset);
            email_template_params.insert(LibFred::Mailer::Parameters::value_type("type", "4"));
            break;
    }
    const Database::Result dbres = ctx.get_conn().exec_params(
            sql,
            Database::query_param_list(handle)(object_type_handle));
    if (dbres.size() < 1)
    {
        throw PublicRequestImpl::NoContactEmail();
    }

    email_template_params.insert(LibFred::Mailer::Parameters::value_type("authinfo", static_cast<std::string>(dbres[0][0])));
    std::set<std::string> recipients;
    for (unsigned idx = 0; idx < dbres.size(); ++idx)
    {
        recipients.insert(static_cast<std::string>(dbres[idx][1]));
    }
    const EmailData data(recipients, "sendauthinfo_pif", email_template_params, std::vector<unsigned long long>());
    return send_joined_addresses_email(manager, data);
}

unsigned long long get_id_of_registered_object(
        LibFred::OperationContext& ctx,
        PublicRequestImpl::ObjectType::Enum object_type,
        const std::string& handle)
{
    switch (object_type)
    {
        case PublicRequestImpl::ObjectType::contact:
            return LibFred::get_id_of_registered<LibFred::Object_Type::contact>(ctx, handle);
        case PublicRequestImpl::ObjectType::nsset:
            return LibFred::get_id_of_registered<LibFred::Object_Type::nsset>(ctx, handle);
        case PublicRequestImpl::ObjectType::domain:
            return LibFred::get_id_of_registered<LibFred::Object_Type::domain>(ctx, handle);
        case PublicRequestImpl::ObjectType::keyset:
            return LibFred::get_id_of_registered<LibFred::Object_Type::keyset>(ctx, handle);
    }
    throw std::logic_error("unexpected PublicRequestImpl::ObjectType::Enum value");
}

void check_authinfo_request_permission(const LibFred::ObjectStatesInfo& states)
{
    if (states.presents(LibFred::Object_State::server_transfer_prohibited))
    {
        throw PublicRequestImpl::ObjectTransferProhibited();
    }
}

} // namespace Fred::Backend::PublicRequest::{anonymous}

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
        const Optional<unsigned long long>& log_request_id,
        std::shared_ptr<LibFred::Mailer::Manager> manager) const // potentially put as member
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        unsigned long long object_id;
        unsigned long long public_request_id;
        const Type::AuthinfoAuto public_request_type;
        {
            LibFred::OperationContextCreator ctx;
            object_id = get_id_of_registered_object(ctx, object_type, object_handle);
            const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(object_id).exec(ctx));
            check_authinfo_request_permission(states);
            LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, object_id);
            public_request_id = LibFred::CreatePublicRequest(
                    "create_authinfo_request_registry_email call",
                    Optional<std::string>(),
                    Optional<unsigned long long>())
                                        .exec(locked_object, public_request_type, log_request_id);
            ctx.commit_transaction();
        }
        try
        {
            const unsigned long long email_id =
                    send_authinfo(public_request_id, object_handle, object_type, manager);
            try
            {
                LibFred::OperationContextCreator ctx;
                LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, object_id);
                LibFred::UpdatePublicRequest(
                        LibFred::PublicRequest::Status::resolved,
                        Optional<Nullable<std::string> >(),
                        Optional<Nullable<std::string> >(),
                        email_id,
                        Optional<Nullable<LibFred::RegistrarId> >())
                        .exec(locked_object, public_request_type, log_request_id);
                ctx.commit_transaction();
            }
            catch (...)
            {
                LOGGER(PACKAGE).info(boost::format("Request %1% update failed, but email %2% sent") % public_request_id % email_id);
                //no throw as main part is completed
            }
            return public_request_id;
        }
        catch (...)
        {
            LibFred::OperationContextCreator ctx;
            LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, object_id);
            LibFred::UpdatePublicRequest(
                    LibFred::PublicRequest::Status::invalidated,
                    Optional<Nullable<std::string> >(),
                    Optional<Nullable<std::string> >(),
                    Optional<Nullable<unsigned long long> >(),
                    Optional<Nullable<LibFred::RegistrarId> >())
                    .exec(locked_object, public_request_type, log_request_id);
            ctx.commit_transaction();
            throw;
        }
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
    catch (const ObjectTransferProhibited& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectTransferProhibited();
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
        LOGGER(PACKAGE).error("Unknown error");
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
        const unsigned long long object_id = get_id_of_registered_object(ctx, object_type, object_handle);
        const LibFred::ObjectStatesInfo states(LibFred::GetObjectStates(object_id).exec(ctx));
        check_authinfo_request_permission(states);
        LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, object_id);
        const LibFred::CreatePublicRequest create_public_request_op(
                "create_authinfo_request_non_registry_email call",
                specified_email,
                Optional<unsigned long long>());
        switch (confirmation_method)
        {
            case ConfirmedBy::email:
            {
                const unsigned long long request_id =
                        create_public_request_op.exec(locked_object, Type::AuthinfoEmail(), log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case ConfirmedBy::letter:
            {
                const unsigned long long request_id =
                        create_public_request_op.exec(locked_object, Type::AuthinfoPost(), log_request_id);
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
    catch (const ObjectTransferProhibited& e)
    {
        LOGGER(PACKAGE).info(e.what());
        throw ObjectTransferProhibited();
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

namespace {

template <typename request>
const LibFred::PublicRequestTypeIface& get_public_request_type(PublicRequestImpl::ConfirmedBy::Enum confirmation_method)
{
    switch (confirmation_method)
    {
        case PublicRequestImpl::ConfirmedBy::email:
        {
            static const typename request::ByEmail public_request_type;
            return static_cast<const LibFred::PublicRequestTypeIface&>(public_request_type);
        }
        case PublicRequestImpl::ConfirmedBy::letter:
        {
            static const typename request::ByPost public_request_type;
            return static_cast<const LibFred::PublicRequestTypeIface&>(public_request_type);
        }
    }
    throw std::runtime_error("unexpected confirmation method");
}

} // namespace Fred::Backend::PublicRequest::{anonymous}

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
                                get_public_request_type<Type::Block::Transfer>(confirmation_method),
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
                                get_public_request_type<Type::Block::Changes>(confirmation_method),
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
                                get_public_request_type<Type::Unblock::Transfer>(confirmation_method),
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
                                get_public_request_type<Type::Unblock::Changes>(confirmation_method),
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

namespace {

unsigned long long get_id_of_contact(LibFred::OperationContext& ctx, const std::string& contact_handle)
{
    return get_id_of_registered_object(ctx, PublicRequestImpl::ObjectType::contact, contact_handle);
}

} // namespace Fred::Backend::PublicRequest::Type::{anonymous}

unsigned long long PublicRequestImpl::create_personal_info_request_registry_email(
        const std::string& contact_handle,
        const Optional<unsigned long long>& log_request_id,
        std::shared_ptr<LibFred::Mailer::Manager> manager) const
{
    LOGGING_CONTEXT(log_ctx, *this);
    try
    {
        LibFred::OperationContextCreator ctx;
        const auto contact_id = get_id_of_contact(ctx, contact_handle);
        LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, contact_id);
        const auto public_request_id = LibFred::CreatePublicRequest()
            .exec(locked_object, Type::PersonalinfoAuto(), log_request_id);
        LibFred::UpdatePublicRequest()
            .set_status(LibFred::PublicRequest::Status::resolved)
            .exec(locked_object, Type::PersonalinfoAuto(), log_request_id);
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
        const unsigned long long contact_id = get_id_of_contact(ctx, contact_handle);
        LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_object(ctx, contact_id);
        const auto create_public_request_op = LibFred::CreatePublicRequest().set_email_to_answer(specified_email);
        switch (confirmation_method)
        {
            case ConfirmedBy::email:
            {
                const unsigned long long request_id =
                        create_public_request_op.exec(locked_object, Type::PersonalinfoEmail(), log_request_id);
                ctx.commit_transaction();
                return request_id;
            }
            case ConfirmedBy::letter:
            {
                const unsigned long long request_id =
                        create_public_request_op.exec(locked_object, Type::PersonalinfoPost(), log_request_id);
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

namespace {

std::map<std::string, unsigned char> get_public_request_type_to_post_type_dictionary()
{
    std::map<std::string, unsigned char> dictionary;
    if (dictionary.insert(std::make_pair(Type::AuthinfoPost().get_public_request_type(), 1)).second &&
            dictionary.insert(std::make_pair(Type::Block::Transfer::ByPost().get_public_request_type(), 2)).second &&
            dictionary.insert(std::make_pair(Type::Unblock::Transfer::ByPost().get_public_request_type(), 3)).second &&
            dictionary.insert(std::make_pair(Type::Block::Changes::ByPost().get_public_request_type(), 4)).second &&
            dictionary.insert(std::make_pair(Type::Unblock::Changes::ByPost().get_public_request_type(), 5)).second &&
            dictionary.insert(std::make_pair(Type::PersonalinfoPost().get_public_request_type(), 6)).second)
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
    throw PublicRequestImpl::InvalidPublicRequestType();
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

} // namespace Fred::Backend::PublicRequest::{anonymous}

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

std::shared_ptr<LibFred::Mailer::Manager> PublicRequestImpl::get_default_mailer_manager()
{
    return std::shared_ptr<LibFred::Mailer::Manager>(
            new MailerManager(CorbaContainer::get_instance()->getNS()));
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

const LibFred::PublicRequestTypeIface& get_personal_info_auto_iface()
{
    static const Type::PersonalinfoAuto singleton;
    return singleton;
}

const LibFred::PublicRequestTypeIface& get_personal_info_email_iface()
{
    static const Type::PersonalinfoEmail singleton;
    return singleton;
}

const LibFred::PublicRequestTypeIface& get_personal_info_post_iface()
{
    static const Type::PersonalinfoPost singleton;
    return singleton;
}


} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred
