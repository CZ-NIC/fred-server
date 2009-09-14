#include "model_enum_tlds.h"

std::string ModelEnumTlds::table_name = "enum_tlds";

DEFINE_PRIMARY_KEY(ModelEnumTlds, std::string, tld, m_tld, table_name, "tld", .setDefault())


ModelEnumTlds::field_list ModelEnumTlds::fields = list_of<ModelEnumTlds::field_list::value_type>
    (&ModelEnumTlds::tld)
;

