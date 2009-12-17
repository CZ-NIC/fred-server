#include "model_request.h"

std::string ModelRequest::table_name = "request";

DEFINE_PRIMARY_KEY(ModelRequest, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelRequest, Database::DateTime, timeBegin, m_timeBegin, table_name, "time_begin", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequest, Database::DateTime, timeEnd, m_timeEnd, table_name, "time_end", )
DEFINE_BASIC_FIELD(ModelRequest, std::string, sourceIp, m_sourceIp, table_name, "source_ip", )
DEFINE_BASIC_FIELD(ModelRequest, std::string, userName, m_userName, table_name, "user_name", )
DEFINE_BASIC_FIELD(ModelRequest, unsigned long long, actionTypeId, m_actionTypeId, table_name, "action_type", .setDefault().setForeignKey())
DEFINE_BASIC_FIELD(ModelRequest, unsigned long long, sessionId, m_sessionId, table_name, "session_id", .setForeignKey())
DEFINE_BASIC_FIELD(ModelRequest, unsigned long long, serviceId, m_serviceId, table_name, "service", .setNotNull().setForeignKey())
DEFINE_BASIC_FIELD(ModelRequest, bool, isMonitoring, m_isMonitoring, table_name, "is_monitoring", .setNotNull())

//DEFINE_ONE_TO_ONE(ModelRequest, ModelService, service, m_service, unsigned long long, serviceId, m_serviceId)
//DEFINE_ONE_TO_ONE(ModelRequest, ModelRequestType, actionType, m_actionType, unsigned long long, actionTypeId, m_actionTypeId)
//DEFINE_ONE_TO_ONE(ModelRequest, ModelSession, session, m_session, unsigned long long, sessionId, m_sessionId)

ModelRequest::field_list ModelRequest::fields = list_of<ModelRequest::field_list::value_type>
    (&ModelRequest::id)
    (&ModelRequest::timeBegin)
    (&ModelRequest::timeEnd)
    (&ModelRequest::sourceIp)
    (&ModelRequest::userName)
    (&ModelRequest::serviceId)
    (&ModelRequest::actionTypeId)
    (&ModelRequest::sessionId)
    (&ModelRequest::isMonitoring)
;

