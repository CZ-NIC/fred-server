#include "model_request_property.h"

std::string ModelRequestProperty::table_name = "request_property";

DEFINE_PRIMARY_KEY(ModelRequestProperty, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelRequestProperty, std::string, name, m_name, table_name, "name", .setNotNull())


ModelRequestProperty::field_list ModelRequestProperty::fields = list_of<ModelRequestProperty::field_list::value_type>
    (&ModelRequestProperty::id)
    (&ModelRequestProperty::name)
;

