#include "model_log_property_name.h"

std::string ModelLogPropertyName::table_name = "log_property_name";

DEFINE_PRIMARY_KEY(ModelLogPropertyName, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelLogPropertyName, std::string, name, m_name, table_name, "name", .setNotNull())


ModelLogPropertyName::field_list ModelLogPropertyName::fields = list_of<ModelLogPropertyName::field_list::value_type>
    (&ModelLogPropertyName::id)
    (&ModelLogPropertyName::name)
;

