#include "model_enumfiletype.h"

std::string ModelEnumFileType::table_name = "enum_filetype";

DEFINE_PRIMARY_KEY(ModelEnumFileType, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelEnumFileType, std::string, name, m_name, table_name, "name", )


ModelEnumFileType::field_list ModelEnumFileType::fields = list_of<ModelEnumFileType::field_list::value_type>
    (&ModelEnumFileType::id)
    (&ModelEnumFileType::name)
;

