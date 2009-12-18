#include "model_request_data.h"

std::string ModelRequestData::table_name = "request_data";

DEFINE_PRIMARY_KEY(ModelRequestData, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelRequestData, Database::DateTime, entryTimeBegin, m_entryTimeBegin, table_name, "entry_time_begin", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestData, int, entryService, m_entryService, table_name, "entry_service", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestData, bool, entryMonitoring, m_entryMonitoring, table_name, "entry_monitoring", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestData, unsigned long long, entryId, m_entryId, table_name, "entry_id", .setNotNull().setForeignKey())
DEFINE_BASIC_FIELD(ModelRequestData, std::string, content, m_content, table_name, "content", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequestData, bool, isResponse, m_isResponse, table_name, "is_response", .setDefault())

//DEFINE_ONE_TO_ONE(ModelRequestData, ModelRequest, entry, m_entry, unsigned long long, entryId, m_entryId)

ModelRequestData::field_list ModelRequestData::fields = list_of<ModelRequestData::field_list::value_type>
    (&ModelRequestData::id)
    (&ModelRequestData::entryTimeBegin)
    (&ModelRequestData::entryService)
    (&ModelRequestData::entryMonitoring)
    (&ModelRequestData::entryId)
    (&ModelRequestData::content)
    (&ModelRequestData::isResponse)
;

