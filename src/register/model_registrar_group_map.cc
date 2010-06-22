#include "model_registrar_group_map.h"

std::string ModelRegistrarGroupMap::table_name = "registrar_group_map";

DEFINE_PRIMARY_KEY(ModelRegistrarGroupMap, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelRegistrarGroupMap, unsigned long long, registrar_id, m_registrar_id, table_name, "registrar_id",.setForeignKey() )
DEFINE_BASIC_FIELD(ModelRegistrarGroupMap, unsigned long long, registrar_group_id, m_registrar_group_id, table_name, "registrar_group_id",.setForeignKey() )
DEFINE_BASIC_FIELD(ModelRegistrarGroupMap, Database::Date, member_from, m_member_from, table_name, "member_from", .setNotNull())
DEFINE_BASIC_FIELD(ModelRegistrarGroupMap, Database::Date, member_until, m_member_until, table_name, "member_until", )


ModelRegistrarGroupMap::field_list ModelRegistrarGroupMap::fields = list_of<ModelRegistrarGroupMap::field_list::value_type>
    (&ModelRegistrarGroupMap::id)
    (&ModelRegistrarGroupMap::registrar_id)
    (&ModelRegistrarGroupMap::registrar_group_id)
    (&ModelRegistrarGroupMap::member_from)
    (&ModelRegistrarGroupMap::member_until)
;

