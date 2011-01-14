#include "common_diff.h"

namespace Fred {


void _diff_object(ChangesMap &_changes,
                  const Fred::Object *_prev,
                  const Fred::Object *_act)
{
    compare_and_fill(_changes, "object.authinfo", _prev->getAuthPw(), _act->getAuthPw());
}



std::pair<unsigned long long, unsigned long long>
    get_last_history(const unsigned long long &_id, const unsigned int &_type)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Result history_data = conn.exec_params(
            "SELECT h.id AS prev, h.next AS act FROM history h"
            " WHERE h.next = (SELECT historyid FROM object_registry"
            " WHERE id = $1::integer AND type = $2::integer)",
            Database::query_param_list(_id)(_type));

    if (history_data.size() != 1) {
        throw std::runtime_error("no history found");
    }

    return std::make_pair(static_cast<unsigned long long>(history_data[0][0]),
                          static_cast<unsigned long long>(history_data[0][1]));
}


}

