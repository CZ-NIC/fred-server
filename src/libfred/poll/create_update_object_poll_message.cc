#include "src/libfred/poll/create_update_object_poll_message.hh"
#include "src/libfred/poll/create_poll_message.hh"
#include "src/libfred/object/object_type.hh"
#include "src/libfred/opexception.hh"

namespace LibFred {
namespace Poll {

void CreateUpdateObjectPollMessage::exec(LibFred::OperationContext &_ctx, unsigned long long _history_id)const
{
    const Database::Result db_res = _ctx.get_conn().exec_params(
            "SELECT eot.name "
            "FROM object_registry obr "
            "JOIN object_history oh ON oh.id=obr.id "
            "JOIN enum_object_type eot ON eot.id=obr.type "
            "WHERE oh.historyid=$1::BIGINT",
            Database::query_param_list(_history_id));

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
    switch (Conversion::Enums::from_db_handle<Object_Type>(static_cast<std::string>(db_res[0][0])))
    {
        case Object_Type::contact:
            CreatePollMessage<MessageType::update_contact>().exec(_ctx, _history_id);
            return;
        case Object_Type::domain:
            CreatePollMessage<MessageType::update_domain>().exec(_ctx, _history_id);
            return;
        case Object_Type::keyset:
            CreatePollMessage<MessageType::update_keyset>().exec(_ctx, _history_id);
            return;
        case Object_Type::nsset:
            CreatePollMessage<MessageType::update_nsset>().exec(_ctx, _history_id);
            return;
    }
    struct UnexpectedObjectType:InternalError
    {
        UnexpectedObjectType():InternalError("unexpected object type") { }
    };
    throw UnexpectedObjectType();
}


} // namespace LibFred::Poll
} // namespace LibFred
