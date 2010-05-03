#include "model_registrar_group.h"

std::string ModelRegistrarGroup::table_name = "registrar_group";

DEFINE_PRIMARY_KEY(ModelRegistrarGroup, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelRegistrarGroup, std::string, short_name, m_short_name, table_name, "short_name", .setNotNull().setUnique() )
DEFINE_BASIC_FIELD(ModelRegistrarGroup, Database::DateTime, cancelled, m_cancelled, table_name, "cancelled", )

ModelRegistrarGroup::field_list ModelRegistrarGroup::fields = list_of<ModelRegistrarGroup::field_list::value_type>
    (&ModelRegistrarGroup::id)
    (&ModelRegistrarGroup::short_name)
    (&ModelRegistrarGroup::cancelled)
;

