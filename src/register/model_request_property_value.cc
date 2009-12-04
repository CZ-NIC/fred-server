#include "model_request_property_value.h"

std::string ModelRequestPropertyValue::table_name = "request_property_value";

DEFINE_BASIC_FIELD(ModelRequestPropertyValue, Database::DateTime, entryTimeBegin, m_entryTimeBegin, table_name, "entry_time_begin", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestPropertyValue, int, entryService, m_entryService, table_name, "entry_service", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestPropertyValue, bool, entryMonitoring, m_entryMonitoring, table_name, "entry_monitoring", .setNotNull())
DEFINE_PRIMARY_KEY(ModelRequestPropertyValue, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelRequestPropertyValue, int, entry, m_entry, table_name, "entry_id", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestPropertyValue, int, name, m_name, table_name, "name_id", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestPropertyValue, std::string, value, m_value, table_name, "value", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestPropertyValue, bool, output, m_output, table_name, "output", .setDefault())
DEFINE_BASIC_FIELD(ModelRequestPropertyValue, unsigned long long, parentId, m_parentId, table_name, "parent_id",.setForeignKey() )

//DEFINE_ONE_TO_ONE(ModelRequestPropertyValue, ModelRequestPropertyValue, parent, m_parent, unsigned long long, parentId, m_parentId)

ModelRequestPropertyValue::field_list ModelRequestPropertyValue::fields = list_of<ModelRequestPropertyValue::field_list::value_type>
    (&ModelRequestPropertyValue::entryTimeBegin)
    (&ModelRequestPropertyValue::entryService)
    (&ModelRequestPropertyValue::entryMonitoring)
    (&ModelRequestPropertyValue::id)
    (&ModelRequestPropertyValue::entry)
    (&ModelRequestPropertyValue::name)
    (&ModelRequestPropertyValue::value)
    (&ModelRequestPropertyValue::output)
    (&ModelRequestPropertyValue::parentId)
;

