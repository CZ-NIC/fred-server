#include "model_request_data.h"

std::string ModelRequestData::table_name = "request_data";

DEFINE_PRIMARY_KEY(ModelRequestData, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelRequestData, Database::DateTime, requestTimeBegin, m_requestTimeBegin, table_name, "request_time_begin", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestData, int, requestServiceId, m_requestServiceId, table_name, "request_service_id", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestData, bool, requestMonitoring, m_requestMonitoring, table_name, "request_monitoring", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestData, unsigned long long, requestId, m_requestId, table_name, "request_id", .setNotNull().setForeignKey())
DEFINE_BASIC_FIELD(ModelRequestData, std::string, content, m_content, table_name, "content", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestData, bool, isResponse, m_isResponse, table_name, "is_response", .setDefault())

//DEFINE_ONE_TO_ONE(ModelRequestData, ModelRequest, request, m_request, unsigned long long, requestId, m_requestId)

ModelRequestData::field_list ModelRequestData::fields = list_of<ModelRequestData::field_list::value_type>
    (&ModelRequestData::id)
    (&ModelRequestData::requestTimeBegin)
    (&ModelRequestData::requestServiceId)
    (&ModelRequestData::requestMonitoring)
    (&ModelRequestData::requestId)
    (&ModelRequestData::content)
    (&ModelRequestData::isResponse)
;

