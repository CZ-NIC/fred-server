#include "model_enumoperation.h"

std::string ModelEnumOperation::table_name = "enum_operation";

DEFINE_PRIMARY_KEY(ModelEnumOperation, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelEnumOperation, std::string, operation, m_operation, table_name, "operation", .setNotNull())

ModelEnumOperation::field_list ModelEnumOperation::fields = list_of<ModelEnumOperation::field_list::value_type>
    (&ModelEnumOperation::id)
    (&ModelEnumOperation::operation);
