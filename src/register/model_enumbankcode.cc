#include "model_enumbankcode.h"

std::string ModelEnumBankCode::table_name = "enum_bank_code";

DEFINE_PRIMARY_KEY(ModelEnumBankCode, std::string, code, m_code, table_name, "code", .setDefault())
DEFINE_BASIC_FIELD(ModelEnumBankCode, std::string, nameShort, m_nameShort, table_name, "name_short", .setNotNull())
DEFINE_BASIC_FIELD(ModelEnumBankCode, std::string, nameFull, m_nameFull, table_name, "name_full", .setNotNull())


ModelEnumBankCode::field_list ModelEnumBankCode::fields = list_of<ModelEnumBankCode::field_list::value_type>
    (&ModelEnumBankCode::code)
    (&ModelEnumBankCode::nameShort)
    (&ModelEnumBankCode::nameFull)
;

