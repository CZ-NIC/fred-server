#include "model_log_raw_content.h"

std::string ModelLogRawContent::table_name = "log_raw_content";

DEFINE_BASIC_FIELD(ModelLogRawContent, Database::DateTime, entryTimeBegin, m_entryTimeBegin, table_name, "entry_time_begin", .setNotNull())
DEFINE_FOREIGN_KEY(ModelLogRawContent, ModelLogEntry, unsigned long long, entryId, m_entryId, table_name, "entry_id", id, .setNotNull())
DEFINE_BASIC_FIELD(ModelLogRawContent, std::string, content, m_content, table_name, "content", .setNotNull())
DEFINE_BASIC_FIELD(ModelLogRawContent, bool, isResponse, m_isResponse, table_name, "is_response", .setDefault())

DEFINE_ONE_TO_ONE(ModelLogRawContent, ModelLogEntry, entry, m_entry, unsigned long long, entryId, m_entryId)

ModelLogRawContent::field_list ModelLogRawContent::fields = list_of<ModelLogRawContent::field_list::value_type>
    (&ModelLogRawContent::entryTimeBegin)
    (&ModelLogRawContent::entryId)
    (&ModelLogRawContent::content)
    (&ModelLogRawContent::isResponse)
;

