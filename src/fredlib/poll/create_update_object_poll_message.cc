#include "create_update_object_poll_message.h"
#include "create_poll_message.h"

#include <boost/lexical_cast.hpp>


#define MY_EXCEPTION_CLASS(DATA) CreateUpdateObjectPollMessageException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
#define MY_ERROR_CLASS(DATA)     CreateUpdateObjectPollMessageError(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

namespace Fred {
namespace Poll {


CreateUpdateObjectPollMessage::CreateUpdateObjectPollMessage(
        const ObjectHistoryId &_history_id)
    : history_id_(_history_id)
{
}


void CreateUpdateObjectPollMessage::exec(Fred::OperationContext &_ctx)
{
    try
    {
        Database::Result r = _ctx.get_conn().exec_params(
                "SELECT eot.name AS object_type, r.handle as registrar_handle"
                " FROM object_registry oreg JOIN object_history oh ON oh.id = oreg.id"
                " JOIN enum_object_type eot ON eot.id = oreg.type"
                " JOIN registrar r ON r.id = oh.clid"
                " WHERE oh.historyid = $1::bigint",
                Database::query_param_list(history_id_));

        if (r.size() != 1) {
            std::string errmsg("|| not found:object history:");
            errmsg += boost::lexical_cast<std::string>(history_id_);
            errmsg += " |";
            throw MY_EXCEPTION_CLASS(errmsg.c_str());
        }
        std::string object_type = static_cast<std::string>(r[0][0]);
        std::string sponsoring_registrar = static_cast<std::string>(r[0][1]);
        unsigned long long poll_msg_id = CreatePollMessage(sponsoring_registrar, std::string("update_" + object_type)).exec(_ctx);

        _ctx.get_conn().exec_params(
                "INSERT INTO poll_eppaction (msgid, objid) VALUES ($1::bigint, $2::bigint)",
                Database::query_param_list(poll_msg_id)(history_id_));
    }
    catch (...)
    {
        handleOperationExceptions<CreateUpdateObjectPollMessageException>(
                __FILE__, __LINE__, __ASSERT_FUNCTION);
    }
}


}
}
