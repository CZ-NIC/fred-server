#include "model_log_entry.h"

std::string ModelLogEntry::table_name = "log_entry";

DEFINE_PRIMARY_KEY(ModelLogEntry, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelLogEntry, Database::DateTime, timeBegin, m_timeBegin, table_name, "time_begin", .setNotNull())
DEFINE_BASIC_FIELD(ModelLogEntry, Database::DateTime, timeEnd, m_timeEnd, table_name, "time_end", )
DEFINE_BASIC_FIELD(ModelLogEntry, std::string, sourceIp, m_sourceIp, table_name, "source_ip", )
DEFINE_BASIC_FIELD(ModelLogEntry, int, service, m_service, table_name, "service", .setNotNull())
DEFINE_FOREIGN_KEY(ModelLogEntry, ModelLogActionType, unsigned long long, actionTypeId, m_actionTypeId, table_name, "action_type", id, .setDefault())
DEFINE_BASIC_FIELD(ModelLogEntry, bool, isMonitoring, m_isMonitoring, table_name, "is_monitoring", .setNotNull())

DEFINE_ONE_TO_ONE(ModelLogEntry, ModelLogActionType, actionType, m_actionType, unsigned long long, actionTypeId, m_actionTypeId)

ModelLogEntry::field_list ModelLogEntry::fields = list_of<ModelLogEntry::field_list::value_type>
    (&ModelLogEntry::id)
    (&ModelLogEntry::timeBegin)
    (&ModelLogEntry::timeEnd)
    (&ModelLogEntry::sourceIp)
    (&ModelLogEntry::service)
    (&ModelLogEntry::actionTypeId)
    (&ModelLogEntry::isMonitoring)
;

