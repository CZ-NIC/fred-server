#include "model_service.h"

std::string ModelService::table_name = "service";

DEFINE_PRIMARY_KEY(ModelService, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelService, std::string, name, m_name, table_name, "name", .setNotNull())


ModelService::field_list ModelService::fields = list_of<ModelService::field_list::value_type>
    (&ModelService::id)
    (&ModelService::name)
;

