#include "model_request_property_value.h"

std::string ModelRequestPropertyValue::table_name = "request_property_value";

DEFINE_BASIC_FIELD(ModelRequestPropertyValue, Database::DateTime, requestTimeBegin, m_requestTimeBegin, table_name, "request_time_begin", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestPropertyValue, int, requestServiceId, m_requestServiceId, table_name, "request_service_id", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestPropertyValue, bool, requestMonitoring, m_requestMonitoring, table_name, "request_monitoring", .setNotNull())
DEFINE_PRIMARY_KEY(ModelRequestPropertyValue, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelRequestPropertyValue, unsigned long long, requestId, m_requestId, table_name, "request_id", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestPropertyValue, int, propertyNameId, m_propertyNameId, table_name, "property_name_id", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestPropertyValue, std::string, value, m_value, table_name, "value", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestPropertyValue, bool, output, m_output, table_name, "output", .setDefault())
DEFINE_BASIC_FIELD(ModelRequestPropertyValue, unsigned long long, parentId, m_parentId, table_name, "parent_id",.setForeignKey() )

//DEFINE_ONE_TO_ONE(ModelRequestPropertyValue, ModelRequestPropertyValue, parent, m_parent, unsigned long long, parentId, m_parentId)

ModelRequestPropertyValue::field_list ModelRequestPropertyValue::fields = list_of<ModelRequestPropertyValue::field_list::value_type>
    (&ModelRequestPropertyValue::requestTimeBegin)
    (&ModelRequestPropertyValue::requestServiceId)
    (&ModelRequestPropertyValue::requestMonitoring)
    (&ModelRequestPropertyValue::id)
    (&ModelRequestPropertyValue::requestId)
    (&ModelRequestPropertyValue::propertyNameId)
    (&ModelRequestPropertyValue::value)
    (&ModelRequestPropertyValue::output)
    (&ModelRequestPropertyValue::parentId)
;

