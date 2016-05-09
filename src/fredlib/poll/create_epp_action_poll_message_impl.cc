#include "src/fredlib/poll/create_epp_action_poll_message_impl.h"
#include "src/fredlib/poll/create_poll_message_impl.h"
#include "src/fredlib/poll/message_types.h"

#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>


namespace Fred {
namespace Poll {

CreateEppActionPollMessage::CreateEppActionPollMessage(
    ObjectHistoryId     _history_id,
    object_type         _object_type,
    const std::string&  _message_type_handle
) :
    history_id_(_history_id),
    object_type_(_object_type),
    message_type_handle_(_message_type_handle)
{ }


unsigned long long CreateEppActionPollMessage::exec(Fred::OperationContext &_ctx)
{
    const bool message_to_previous_registrar = (message_type_handle_ == TRANSFER_CONTACT) ||
                                               (message_type_handle_ == TRANSFER_DOMAIN)  ||
                                               (message_type_handle_ == TRANSFER_NSSET)   ||
                                               (message_type_handle_ == TRANSFER_KEYSET);
    Database::Result registrar_and_type_res =
        message_to_previous_registrar
        ? _ctx.get_conn().exec_params(
              "SELECT eot.name AS object_type_,"
                     "r.handle AS registrar_handle_ "
              "FROM history h "
              "JOIN object_history oh ON oh.historyid=h.id "
              "JOIN object_registry oreg ON oreg.id=oh.id "
              "JOIN enum_object_type eot ON eot.id=oreg.type "
              "JOIN registrar r ON r.id=oh.clid "
              "WHERE h.next=$1::BIGINT", Database::query_param_list(history_id_))
        : _ctx.get_conn().exec_params(
              "SELECT eot.name AS object_type_,"
                     "r.handle AS registrar_handle_ "
              "FROM object_registry oreg "
              "JOIN object_history oh ON oh.id=oreg.id "
              "JOIN enum_object_type eot ON eot.id=oreg.type "
              "JOIN registrar r ON r.id=oh.clid "
              "WHERE oh.historyid=$1::BIGINT", Database::query_param_list(history_id_));

    if (registrar_and_type_res.size() != 1) {
        BOOST_THROW_EXCEPTION(Exception().set_object_history_not_found(history_id_));
    }

    if (static_cast<std::string>(registrar_and_type_res[0]["object_type_"])
        !=
        to_registry_handle(object_type_)
    ) {
        BOOST_THROW_EXCEPTION(
            Exception()
                .set_object_type_not_found(
                    std::make_pair(
                        to_registry_handle(object_type_),
                        history_id_)
            )
        );
    }

    const unsigned long long poll_msg_id = CreatePollMessage(
        static_cast<std::string>(registrar_and_type_res[0]["registrar_handle_"]),
        message_type_handle_
    ).exec(_ctx);

    _ctx.get_conn().exec_params(
        "INSERT "
           "INTO poll_eppaction (msgid, objid) "
           "VALUES ($1::bigint, $2::bigint)",
        Database::query_param_list
            (poll_msg_id)
            (history_id_)
    );

    return poll_msg_id;
}

std::string CreateEppActionPollMessage::to_string() const {
    return Util::format_operation_state(
        "CreateEppActionPollMessage",
        boost::assign::list_of
            (std::make_pair("history_id", boost::lexical_cast<std::string>(history_id_)))
    );
}

}
}
