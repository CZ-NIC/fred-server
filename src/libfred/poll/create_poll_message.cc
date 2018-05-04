#include "src/libfred/poll/create_poll_message.hh"
#include "src/libfred/object/object_type.hh"


namespace LibFred {
namespace Poll {

namespace {

template <MessageType::Enum message_type>
unsigned long long create_poll_eppaction_message(
        LibFred::OperationContext& ctx,
        unsigned long long action_history_id,
        unsigned long long recipient_registrar_id)
{
    const Database::Result db_res = ctx.get_conn().exec_params(
            "WITH create_new_message AS ("
                "INSERT INTO message (clid,crdate,exdate,seen,msgtype) "
                "SELECT $2::BIGINT,NOW(),NOW()+'7DAY'::INTERVAL,false,id "
                "FROM messagetype "
                "WHERE name=$3::TEXT "
                "RETURNING id AS msgid) "
            "INSERT INTO poll_eppaction (msgid,objid) "
            "SELECT msgid,$1::BIGINT FROM create_new_message "
            "RETURNING msgid",
            Database::query_param_list(action_history_id)
                                      (recipient_registrar_id)
                                      (Conversion::Enums::to_db_handle(message_type)));
    if (db_res.size() == 1)
    {
        return static_cast<unsigned long long>(db_res[0][0]);
    }
    struct UnexpectedNumberOfRows:InternalError
    {
        UnexpectedNumberOfRows():InternalError("unexpected number of rows") { }
    };
    throw UnexpectedNumberOfRows();
}

struct SponsoringRegistrar
{
    enum Enum
    {
        who_did_the_action,
        at_the_transfer_start,
    };
};

typedef unsigned long long (*CreatePollMessageFunction)(
        LibFred::OperationContext&,
        unsigned long long,
        unsigned long long);

template <MessageType::Enum message_type>
struct MessageTypeTraits { };

template <>
struct MessageTypeTraits<MessageType::transfer_contact>
{
    static const MessageType::Enum message_type = MessageType::transfer_contact;
    static const Object_Type::Enum object_type = Object_Type::contact;
    static const SponsoringRegistrar::Enum recipient = SponsoringRegistrar::at_the_transfer_start;
    static const CreatePollMessageFunction create_poll_message;
};

const CreatePollMessageFunction MessageTypeTraits<MessageType::transfer_contact>::create_poll_message =
        create_poll_eppaction_message<message_type>;

template <>
struct MessageTypeTraits<MessageType::transfer_domain>
{
    static const MessageType::Enum message_type = MessageType::transfer_domain;
    static const Object_Type::Enum object_type = Object_Type::domain;
    static const SponsoringRegistrar::Enum recipient = SponsoringRegistrar::at_the_transfer_start;
    static const CreatePollMessageFunction create_poll_message;
};

const CreatePollMessageFunction MessageTypeTraits<MessageType::transfer_domain>::create_poll_message =
        create_poll_eppaction_message<message_type>;

template <>
struct MessageTypeTraits<MessageType::transfer_nsset>
{
    static const MessageType::Enum message_type = MessageType::transfer_nsset;
    static const Object_Type::Enum object_type = Object_Type::nsset;
    static const SponsoringRegistrar::Enum recipient = SponsoringRegistrar::at_the_transfer_start;
    static const CreatePollMessageFunction create_poll_message;
};

const CreatePollMessageFunction MessageTypeTraits<MessageType::transfer_nsset>::create_poll_message =
        create_poll_eppaction_message<message_type>;

template <>
struct MessageTypeTraits<MessageType::transfer_keyset>
{
    static const MessageType::Enum message_type = MessageType::transfer_keyset;
    static const Object_Type::Enum object_type = Object_Type::keyset;
    static const SponsoringRegistrar::Enum recipient = SponsoringRegistrar::at_the_transfer_start;
    static const CreatePollMessageFunction create_poll_message;
};

const CreatePollMessageFunction MessageTypeTraits<MessageType::transfer_keyset>::create_poll_message =
        create_poll_eppaction_message<message_type>;

template <>
struct MessageTypeTraits<MessageType::update_contact>
{
    static const MessageType::Enum message_type = MessageType::update_contact;
    static const Object_Type::Enum object_type = Object_Type::contact;
    static const SponsoringRegistrar::Enum recipient = SponsoringRegistrar::who_did_the_action;
    static const CreatePollMessageFunction create_poll_message;
};

const CreatePollMessageFunction MessageTypeTraits<MessageType::update_contact>::create_poll_message =
        create_poll_eppaction_message<message_type>;

template <>
struct MessageTypeTraits<MessageType::update_domain>
{
    static const MessageType::Enum message_type = MessageType::update_domain;
    static const Object_Type::Enum object_type = Object_Type::domain;
    static const SponsoringRegistrar::Enum recipient = SponsoringRegistrar::who_did_the_action;
    static const CreatePollMessageFunction create_poll_message;
};

const CreatePollMessageFunction MessageTypeTraits<MessageType::update_domain>::create_poll_message =
        create_poll_eppaction_message<message_type>;

template <>
struct MessageTypeTraits<MessageType::update_nsset>
{
    static const MessageType::Enum message_type = MessageType::update_nsset;
    static const Object_Type::Enum object_type = Object_Type::nsset;
    static const SponsoringRegistrar::Enum recipient = SponsoringRegistrar::who_did_the_action;
    static const CreatePollMessageFunction create_poll_message;
};

const CreatePollMessageFunction MessageTypeTraits<MessageType::update_nsset>::create_poll_message =
        create_poll_eppaction_message<message_type>;

template <>
struct MessageTypeTraits<MessageType::update_keyset>
{
    static const MessageType::Enum message_type = MessageType::update_keyset;
    static const Object_Type::Enum object_type = Object_Type::keyset;
    static const SponsoringRegistrar::Enum recipient = SponsoringRegistrar::who_did_the_action;
    static const CreatePollMessageFunction create_poll_message;
};

const CreatePollMessageFunction MessageTypeTraits<MessageType::update_keyset>::create_poll_message =
        create_poll_eppaction_message<message_type>;

template <>
struct MessageTypeTraits<MessageType::delete_contact>
{
    static const MessageType::Enum message_type = MessageType::delete_contact;
    static const Object_Type::Enum object_type = Object_Type::contact;
    static const SponsoringRegistrar::Enum recipient = SponsoringRegistrar::who_did_the_action;
    static const CreatePollMessageFunction create_poll_message;
};

const CreatePollMessageFunction MessageTypeTraits<MessageType::delete_contact>::create_poll_message =
        create_poll_eppaction_message<message_type>;

template <>
struct MessageTypeTraits<MessageType::delete_domain>
{
    static const MessageType::Enum message_type = MessageType::delete_domain;
    static const Object_Type::Enum object_type = Object_Type::domain;
    static const SponsoringRegistrar::Enum recipient = SponsoringRegistrar::who_did_the_action;
    static const CreatePollMessageFunction create_poll_message;
};

const CreatePollMessageFunction MessageTypeTraits<MessageType::delete_domain>::create_poll_message =
        create_poll_eppaction_message<message_type>;

template <SponsoringRegistrar::Enum recipient>
struct GetRecipient { };

template <>
struct GetRecipient<SponsoringRegistrar::who_did_the_action>
{
    static const char sql[];
};

template <>
struct GetRecipient<SponsoringRegistrar::at_the_transfer_start>
{
    static const char sql[];
};

const char GetRecipient<SponsoringRegistrar::who_did_the_action>::sql[] =
        "SELECT eot.id IS NOT NULL,oh.clid "
        "FROM object_history oh "
        "JOIN object_registry obr ON obr.id=oh.id "
        "LEFT JOIN enum_object_type eot ON eot.id=obr.type AND eot.name=$2::TEXT "
        "WHERE oh.historyid=$1::BIGINT";

const char GetRecipient<SponsoringRegistrar::at_the_transfer_start>::sql[] =
        "SELECT eot.id IS NOT NULL,oh.clid "
        "FROM object_history oh "
        "JOIN object_registry obr ON obr.id=oh.id "
        "LEFT JOIN enum_object_type eot ON eot.id=obr.type AND eot.name=$2::TEXT "
        "WHERE oh.historyid=(SELECT id FROM history WHERE next=$1::BIGINT)";

} // namespace LibFred::Poll::{anonymous}

template <MessageType::Enum message_type>
unsigned long long CreatePollMessage<message_type>::exec(
        LibFred::OperationContext& _ctx,
        unsigned long long _history_id)const
{
    typedef MessageTypeTraits<message_type> MessageTraits;
    const std::string requested_object_type_handle =
            Conversion::Enums::to_db_handle(MessageTraits::object_type);
    const Database::Result db_res =
            _ctx.get_conn().exec_params(
                    GetRecipient<MessageTraits::recipient>::sql,
                    Database::query_param_list(_history_id)(requested_object_type_handle));

    switch (db_res.size())
    {
        case 0:
        {
            struct NotFound:OperationException
            {
                const char* what()const noexcept { return "object history not found"; }
            };
            throw NotFound();
        }
        case 1:
            break;
        default:
        {
            struct TooManyRows:InternalError
            {
                TooManyRows():InternalError("too many rows") { }
            };
            throw TooManyRows();
        }
    }

    const bool object_type_corresponds_to_message_type = static_cast<bool>(db_res[0][0]);
    if (!object_type_corresponds_to_message_type)
    {
        struct NotCorrespondingObjectType:OperationException
        {
            const char* what()const noexcept
            {
                return "associated object is not of the type corresponding to the given message type";
            }
        };
        throw NotCorrespondingObjectType();
    }

    const unsigned long long recipient_registrar_id = static_cast<unsigned long long>(db_res[0][1]);
    return MessageTraits::create_poll_message(_ctx, _history_id, recipient_registrar_id);
}

template struct CreatePollMessage<MessageType::transfer_contact>;
template struct CreatePollMessage<MessageType::transfer_domain>;
template struct CreatePollMessage<MessageType::transfer_nsset>;
template struct CreatePollMessage<MessageType::transfer_keyset>;

template struct CreatePollMessage<MessageType::update_contact>;
template struct CreatePollMessage<MessageType::update_domain>;
template struct CreatePollMessage<MessageType::update_nsset>;
template struct CreatePollMessage<MessageType::update_keyset>;

template struct CreatePollMessage<MessageType::delete_contact>;
template struct CreatePollMessage<MessageType::delete_domain>;

} // namespace LibFred::Poll
} // namespace LibFred
