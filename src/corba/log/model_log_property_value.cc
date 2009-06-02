#include "model_log_property_value.h"

std::string ModelLogPropertyValue::table_name = "log_property_value";

DEFINE_BASIC_FIELD(ModelLogPropertyValue, Database::DateTime, entryTimeBegin, m_entryTimeBegin, table_name, "entry_time_begin", .setNotNull())
DEFINE_PRIMARY_KEY(ModelLogPropertyValue, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_FOREIGN_KEY(ModelLogPropertyValue, ModelLogEntry, unsigned long long, entryId, m_entryId, table_name, "entry_id", id, .setNotNull())
DEFINE_FOREIGN_KEY(ModelLogPropertyValue, ModelLogPropertyName, unsigned long long, nameId, m_nameId, table_name, "name_id", id, .setNotNull())
DEFINE_BASIC_FIELD(ModelLogPropertyValue, std::string, value, m_value, table_name, "value", .setNotNull())
DEFINE_BASIC_FIELD(ModelLogPropertyValue, bool, output, m_output, table_name, "output", .setDefault())
DEFINE_FOREIGN_KEY(ModelLogPropertyValue, ModelLogPropertyValue, unsigned long long, parentId, m_parentId, table_name, "parent_id", id, )

DEFINE_ONE_TO_ONE(ModelLogPropertyValue, ModelLogEntry, entry, m_entry, unsigned long long, entryId, m_entryId)
DEFINE_ONE_TO_ONE(ModelLogPropertyValue, ModelLogPropertyName, name, m_name, unsigned long long, nameId, m_nameId)
DEFINE_ONE_TO_ONE(ModelLogPropertyValue, ModelLogPropertyValue, parent, m_parent, unsigned long long, parentId, m_parentId)

ModelLogPropertyValue::field_list ModelLogPropertyValue::fields = list_of<ModelLogPropertyValue::field_list::value_type>
    (&ModelLogPropertyValue::entryTimeBegin)
    (&ModelLogPropertyValue::id)
    (&ModelLogPropertyValue::entryId)
    (&ModelLogPropertyValue::nameId)
    (&ModelLogPropertyValue::value)
    (&ModelLogPropertyValue::output)
    (&ModelLogPropertyValue::parentId)
;

