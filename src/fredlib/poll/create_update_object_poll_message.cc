#include "src/fredlib/poll/create_update_object_poll_message.h"
#include "src/fredlib/poll/create_epp_action_poll_message.h"

namespace Fred {
namespace Poll {

void CreateUpdateObjectPollMessage::exec(Fred::OperationContext &_ctx, unsigned long long _history_id)const
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
    switch (Conversion::Enums::from_db_handle<Object_Type>(static_cast<std::string>(db_res[0][0])))
    {
        case Object_Type::contact:
            struct ContactsNotSupported:Exception, Exception::ObjectOfUnsupportedType
            {
                const char* what()const throw() { return "contacts not supported"; }
            };
            throw ContactsNotSupported();
        case Object_Type::domain:
            CreateEppActionPollMessage::Of<MessageType::update_domain>().exec(_ctx, _history_id);
            break;
        case Object_Type::keyset:
            CreateEppActionPollMessage::Of<MessageType::update_keyset>().exec(_ctx, _history_id);
            break;
        case Object_Type::nsset:
            CreateEppActionPollMessage::Of<MessageType::update_nsset>().exec(_ctx, _history_id);
            break;
    }
}


}//namespace Fred::Poll
}//namespace Fred
