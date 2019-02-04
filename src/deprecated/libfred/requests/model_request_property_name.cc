#include "src/deprecated/libfred/requests/model_request_property_name.hh"

std::string ModelRequestPropertyName::table_name = "request_property_name";

DEFINE_PRIMARY_KEY(ModelRequestPropertyName, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelRequestPropertyName, std::string, name, m_name, table_name, "name", .setNotNull())


ModelRequestPropertyName::field_list ModelRequestPropertyName::fields = list_of<ModelRequestPropertyName::field_list::value_type>
    (&ModelRequestPropertyName::id)
    (&ModelRequestPropertyName::name)
;

