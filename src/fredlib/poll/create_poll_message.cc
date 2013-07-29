#include "create_poll_message.h"

namespace Fred {
namespace Poll {

CreatePollMessage::CreatePollMessage(
        const std::string &_registrar_handle,
        const std::string &_msg_type)
    : registrar_handle_(_registrar_handle),
      msg_type_(_msg_type)
{
}

unsigned long long CreatePollMessage::exec(Fred::OperationContext &_ctx)
{
    unsigned long long mid = 0;

    //get registrar id
    unsigned long long registrar_id = 0;
    {
        Database::Result registrar_res = _ctx.get_conn().exec_params(
            "SELECT id FROM registrar WHERE handle = UPPER($1::text) FOR SHARE"
            , Database::query_param_list(registrar_handle_));
        if(registrar_res.size() == 0)
        {
            BOOST_THROW_EXCEPTION(Exception().set_registrar_not_found(registrar_handle_));
        }
        if (registrar_res.size() != 1)
        {
            BOOST_THROW_EXCEPTION(InternalError("failed to get registrar"));
        }

        registrar_id = static_cast<unsigned long long>(registrar_res[0][0]);
    }

    //get messagetype id
    unsigned long long messagetype_id = 0;
    {
        Database::Result messagetype_res = _ctx.get_conn().exec_params(
            "SELECT id FROM messagetype WHERE name = $1::text FOR SHARE"
            , Database::query_param_list(msg_type_));
        if(messagetype_res.size() == 0)
        {
            BOOST_THROW_EXCEPTION(Exception().set_poll_message_type_not_found(msg_type_));
        }
        if (messagetype_res.size() != 1)
        {
            BOOST_THROW_EXCEPTION(InternalError("failed to get poll message type"));
        }

        messagetype_id = static_cast<unsigned long long>(messagetype_res[0][0]);
    }

    Database::Result r = _ctx.get_conn().exec_params(
            "INSERT INTO message (clid, msgtype, crdate, exdate)"
            " VALUES ($1::bigint,$2::bigint,"
            " now(), now() + interval '7 day')"
            " RETURNING id",
            Database::query_param_list(registrar_id)(messagetype_id));

    if (r.size() == 1) {
        mid = static_cast<unsigned long long>(r[0][0]);
    }
    else {
        BOOST_THROW_EXCEPTION(InternalError("insert new poll message failed"));
    }

    return mid;
}


}
}

