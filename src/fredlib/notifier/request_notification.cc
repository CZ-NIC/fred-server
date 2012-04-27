#include "request_notification.h"
#include "ntf_except.h"


namespace Fred {


RequestNotification::RequestNotification(const unsigned long long &_request_id)
    : request_id_(_request_id)
{
    if (request_id_ == 0) {
        throw std::runtime_error("create notification: request id not valid");
    }

    Database::Connection conn = Database::Manager::acquire();
    Database::Result info = conn.exec_params(
            "SELECT oreg.id, oreg.name, oreg.type, h1.id AS act, h2.id AS prev"
            " FROM history h1 LEFT join history h2 ON h1.id = h2.next"
            " JOIN object_history oh ON oh.historyid = h1.id"
            " JOIN object_registry oreg ON oreg.id = oh.id"
            " WHERE h1.request_id = $1::bigint",
            Database::query_param_list(_request_id));

    if (info.size() != 1) {
        throw RequestNotFound(_request_id);
    }

    object_id_       = static_cast<unsigned long long>(info[0][0]);
    object_handle_   = static_cast<std::string>(info[0][1]);
    object_type_     = static_cast<unsigned int>(info[0][2]);
    object_hid_act_  = info[0][3].isnull() ? 0 : static_cast<unsigned long long>(info[0][3]);
    object_hid_prev_ = info[0][4].isnull() ? 0 : static_cast<unsigned long long>(info[0][4]);
    load_request_info();
}


void RequestNotification::load_request_info()
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result act_info = conn.exec_params(
            "SELECT oh.historyid, oh.trdate, oh.update, oh.clid, oh.upid"
            " FROM object_history oh"
            " JOIN history h ON h.id = oh.historyid"
            " WHERE h.request_id = $1::bigint",
            Database::query_param_list(request_id_));

    if (act_info.size() != 1) {
        throw std::runtime_error("no such request in object history");
    }

    unsigned long long hid = act_info[0][0];
    Database::Result prev_info = conn.exec_params(
            "SELECT oh.historyid, oh.trdate, oh.update"
            " FROM object_history oh"
            " JOIN history h ON h.id = oh.historyid"
            " WHERE h.next = $1::integer",
            Database::query_param_list(hid));

    if (prev_info.size() != 1) {
        if (object_type_ == 1) {
            request_type_ = CMD_CONTACT_CREATE;
            load_registrar_info(static_cast<unsigned long long>(act_info[0][3]));
            return;
        }
    }

    ptime p_tr = prev_info[0][1];
    ptime a_tr = act_info[0][1];
    ptime p_up = prev_info[0][2];
    ptime a_up = act_info[0][2];

    if (p_tr != a_tr) {
        if (object_type_ == 1) {
            request_type_ = CMD_CONTACT_TRANSFER;
            load_registrar_info(static_cast<unsigned long long>(act_info[0][3]));
            return;
        }
    }
    else if (p_up != a_up) {
        if (object_type_ == 1) {
            request_type_ = CMD_CONTACT_UPDATE;
            load_registrar_info(static_cast<unsigned long long>(act_info[0][4]));
            return;
        }
    }
    throw std::runtime_error("unknown request type");
}


void RequestNotification::load_registrar_info(const unsigned long long _reg_id)
{
    if (_reg_id == 0) {
        throw std::runtime_error("unable to find registrar for request!?");
    }
    Database::Connection conn = Database::Manager::acquire();
    Database::Result rinfo = conn.exec_params(
            "SELECT name || ' (' || url || ')'"
            " FROM registrar WHERE id = $1::integer",
            Database::query_param_list(_reg_id));

    registrar_info_ = static_cast<std::string>(rinfo[0][0]);
}


void RequestNotification::save_relation(const unsigned long long &_msg_id) const
{
    if (_msg_id == 0) {
        throw std::runtime_error("can't save notification without message id");
    }

    Database::Connection conn = Database::Manager::acquire();
    conn.exec_params("INSERT INTO notify_request VALUES ($1::bigint, $2::integer)",
                         Database::query_param_list(request_id_)(_msg_id));
}


}

