#include "src/fredlib/poll/create_epp_action_poll_message.h"
#include "src/fredlib/poll/message_type.h"

#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>


namespace Fred {
namespace Poll {

namespace {

template <MessageType::Enum message_type>
struct MessageTypeTraits { };

template <>
struct MessageTypeTraits<MessageType::transfer_contact>
{
    static const Object_Type::Enum object_type = Object_Type::contact;
    static const bool message_to_previous_registrar = true;
};

template <>
struct MessageTypeTraits<MessageType::transfer_domain>
{
    static const Object_Type::Enum object_type = Object_Type::domain;
    static const bool message_to_previous_registrar = true;
};

template <>
struct MessageTypeTraits<MessageType::transfer_nsset>
{
    static const Object_Type::Enum object_type = Object_Type::nsset;
    static const bool message_to_previous_registrar = true;
};

template <>
struct MessageTypeTraits<MessageType::transfer_keyset>
{
    static const Object_Type::Enum object_type = Object_Type::keyset;
    static const bool message_to_previous_registrar = true;
};

template <>
struct MessageTypeTraits<MessageType::update_domain>
{
    static const Object_Type::Enum object_type = Object_Type::domain;
    static const bool message_to_previous_registrar = false;
};

template <>
struct MessageTypeTraits<MessageType::update_nsset>
{
    static const Object_Type::Enum object_type = Object_Type::nsset;
    static const bool message_to_previous_registrar = false;
};

template <>
struct MessageTypeTraits<MessageType::update_keyset>
{
    static const Object_Type::Enum object_type = Object_Type::keyset;
    static const bool message_to_previous_registrar = false;
};

template <>
struct MessageTypeTraits<MessageType::delete_contact>
{
    static const Object_Type::Enum object_type = Object_Type::contact;
    static const bool message_to_previous_registrar = false;
};

template <>
struct MessageTypeTraits<MessageType::delete_domain>
{
    static const Object_Type::Enum object_type = Object_Type::domain;
    static const bool message_to_previous_registrar = false;
};

template <bool message_to_previous_registrar>
struct GetRecipient { };

template <>
struct GetRecipient<false>
{
    static const char sql[];
};

template <>
struct GetRecipient<true>
{
    static const char sql[];
};

template <>
const char GetRecipient<false>::sql[] =
        "SELECT eot.id IS NOT NULL,oh.clid "
        "FROM object_history oh "
        "JOIN object_registry obr ON obr.id=oh.id "
        "LEFT JOIN enum_object_type eot ON eot.id=obr.type AND eot.name=$2::TEXT "
        "WHERE oh.historyid=$1::BIGINT";

template <>
const char GetRecipient<true>::sql[] =
        "SELECT eot.id IS NOT NULL,oh.clid "
        "FROM history h "
        "JOIN object_history oh ON oh.historyid=h.id "
        "JOIN object_registry obr ON obr.id=oh.id "
        "LEFT JOIN enum_object_type eot ON eot.id=obr.type AND eot.name=$2::TEXT "
        "WHERE h.next=$1::BIGINT";

unsigned long long create_epp_action_poll_message(
        Fred::OperationContext& ctx,
        unsigned long long action_history_id,
        unsigned long long recipient_registrar_id,
        MessageType::Enum message_type)
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
    struct UnexpectedNumberOfRows:CreateEppActionPollMessage::Exception
    {
        const char* what()const throw() { return "unexpected number of rows"; }
    };
    throw UnexpectedNumberOfRows();
}

}//namespace Fred::Poll::{anonymous}

template <MessageType::Enum message_type>
unsigned long long CreateEppActionPollMessage::Of<message_type>::exec(
        Fred::OperationContext& _ctx,
        unsigned long long _history_id)const
{
    const bool message_to_previous_registrar =
            MessageTypeTraits<message_type>::message_to_previous_registrar;
    const std::string requested_object_type_handle =
            Conversion::Enums::to_db_handle(MessageTypeTraits<message_type>::object_type);
    const Database::Result db_res =
            _ctx.get_conn().exec_params(
                    GetRecipient<message_to_previous_registrar>::sql,
                    Database::query_param_list(_history_id)(requested_object_type_handle));

    switch (db_res.size())
    {
        case 0:
        {
            struct NotFound:Exception, Exception::ObjectHistoryNotFound
            {
                NotFound(unsigned long long _history_id):Exception::ObjectHistoryNotFound(_history_id) { }
                const char* what()const throw() { return "object history not found"; }
            };
            throw NotFound(_history_id);
        }
        case 1:
            break;
        default:
        {
            struct TooManyRows:Exception
            {
                const char* what()const throw() { return "too many rows"; }
            };
            throw TooManyRows();
        }
    }

    const bool object_is_requested_type = static_cast<bool>(db_res[0][0]);
    if (!object_is_requested_type)
    {
        struct ObjectNotRequestedType:Exception, Exception::ObjectNotRequestedType
        {
            const char* what()const throw() { return "object not requested type"; }
        };
        throw ObjectNotRequestedType();
    }

    const unsigned long long recipient_registrar_id = static_cast<unsigned long long>(db_res[0][1]);
    return create_epp_action_poll_message(_ctx, _history_id, recipient_registrar_id, message_type);
}

template struct CreateEppActionPollMessage::Of<MessageType::transfer_contact>;
template struct CreateEppActionPollMessage::Of<MessageType::transfer_domain>;
template struct CreateEppActionPollMessage::Of<MessageType::transfer_nsset>;
template struct CreateEppActionPollMessage::Of<MessageType::transfer_keyset>;

template struct CreateEppActionPollMessage::Of<MessageType::update_domain>;
template struct CreateEppActionPollMessage::Of<MessageType::update_nsset>;
template struct CreateEppActionPollMessage::Of<MessageType::update_keyset>;

template struct CreateEppActionPollMessage::Of<MessageType::delete_contact>;
template struct CreateEppActionPollMessage::Of<MessageType::delete_domain>;

}//namespace Fred::Poll
}//namespace Fred
