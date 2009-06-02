#include "model_log_action_type.h"

std::string ModelLogActionType::table_name = "log_action_type";

DEFINE_PRIMARY_KEY(ModelLogActionType, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelLogActionType, std::string, status, m_status, table_name, "status", .setNotNull())


ModelLogActionType::field_list ModelLogActionType::fields = list_of<ModelLogActionType::field_list::value_type>
    (&ModelLogActionType::id)
    (&ModelLogActionType::status)
;

