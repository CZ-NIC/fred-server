#include "model_request_type.h"

std::string ModelRequestType::table_name = "request_type";

DEFINE_PRIMARY_KEY(ModelRequestType, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelRequestType, std::string, status, m_status, table_name, "status", .setNotNull())


ModelRequestType::field_list ModelRequestType::fields = list_of<ModelRequestType::field_list::value_type>
    (&ModelRequestType::id)
    (&ModelRequestType::status)
;

