#include "create_poll_message.h"


#define MY_EXCEPTION_CLASS(DATA) CreatePollMessageException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define MY_ERROR_CLASS(DATA)     CreatePollMessageError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))


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
    try
    {
        Database::Result r = _ctx.get_conn().exec_params(
                "INSERT INTO message (clid, msgtype, crdate, exdate)"
                " VALUES (raise_exception_ifnull((SELECT id FROM registrar WHERE handle = $1::varchar),"
                " '|| not found:registrar: ' || ex_data($1::text) || ' |'),"
                " raise_exception_ifnull((SELECT id FROM messagetype WHERE name = $2::varchar),"
                " '|| not found:poll message type: ' || ex_data($2::text) || ' |'),"
                " now(), now() + interval '7 day')"
                " RETURNING id",
                Database::query_param_list(registrar_handle_)(msg_type_));

        if (r.size() == 1) {
            mid = static_cast<unsigned long long>(r[0][0]);
        }
        else {
            throw MY_ERROR_CLASS("insert new poll messge failed");
        }
    }
    catch (...)
    {
        handleOperationExceptions<CreatePollMessageException>(__FILE__, __LINE__, __ASSERT_FUNCTION);
    }
    return mid;
}


}
}

