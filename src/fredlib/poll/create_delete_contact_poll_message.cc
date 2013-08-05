#include "create_delete_contact_poll_message.h"
#include "create_poll_message.h"

#include <boost/lexical_cast.hpp>


namespace Fred {
namespace Poll {


CreateDeleteContactPollMessage::CreateDeleteContactPollMessage(
        const ObjectHistoryId &_history_id)
    : history_id_(_history_id)
{
}


void CreateDeleteContactPollMessage::exec(Fred::OperationContext &_ctx)
{
    Database::Result r = _ctx.get_conn().exec_params(
            "SELECT eot.name AS object_type, r.handle as registrar_handle"
            " FROM object_registry oreg JOIN object_history oh ON oh.id = oreg.id"
            " JOIN enum_object_type eot ON eot.id = oreg.type"
            " JOIN registrar r ON r.id = oh.clid"
            " WHERE oh.historyid = $1::bigint",
            Database::query_param_list(history_id_));

    if (r.size() != 1) {
        BOOST_THROW_EXCEPTION(Exception().set_object_history_not_found(history_id_));
    }

    if (static_cast<std::string>(r[0][0]) != "contact") {
        BOOST_THROW_EXCEPTION(Exception().set_contact_not_found(history_id_));
    }

    std::string sponsoring_registrar = static_cast<std::string>(r[0][1]);
    unsigned long long poll_msg_id = CreatePollMessage(sponsoring_registrar, std::string("delete_contact")).exec(_ctx);

    _ctx.get_conn().exec_params(
            "INSERT INTO poll_eppaction (msgid, objid) VALUES ($1::bigint, $2::bigint)",
            Database::query_param_list(poll_msg_id)(history_id_));

}


}
}

