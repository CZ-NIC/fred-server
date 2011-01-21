#include "request_notification.h"
#include "fredlib/db_settings.h"


namespace Fred {


RequestNotification::RequestNotification(const unsigned long long &_request_id)
    : request_id_(_request_id),
      object_id_(0)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result info = conn.exec_params(
            "SELECT h.id, oreg.id, oreg.name, oreg.type"
            " FROM history h"
            " JOIN object_history oh ON oh.historyid = h.id"
            " JOIN object_registry oreg ON oreg.id = oh.id"
            " WHERE h.request_id = $1::integer",
            Database::query_param_list(_request_id));

    if (info.size() != 1) {
        throw std::runtime_error("create notification: no such request");
    }

    object_id_ = static_cast<unsigned long long>(info[0][1]);
    object_handle_ = static_cast<std::string>(info[0][2]);
    object_type_ = static_cast<unsigned int>(info[0][3]);
}



}

