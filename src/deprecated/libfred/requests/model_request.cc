#include "src/deprecated/libfred/requests/model_request.hh"

std::string ModelRequest::table_name = "request";

DEFINE_PRIMARY_KEY(ModelRequest, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelRequest, Database::DateTime, timeBegin, m_timeBegin, table_name, "time_begin", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequest, Database::DateTime, timeEnd, m_timeEnd, table_name, "time_end", )
DEFINE_BASIC_FIELD(ModelRequest, std::string, sourceIp, m_sourceIp, table_name, "source_ip", )
DEFINE_BASIC_FIELD(ModelRequest, std::string, userName, m_userName, table_name, "user_name", )
DEFINE_BASIC_FIELD(ModelRequest, unsigned long long, userId, m_userId, table_name, "user_id", )
DEFINE_BASIC_FIELD(ModelRequest, unsigned long long, requestTypeId, m_requestTypeId, table_name, "request_type_id", .setDefault().setForeignKey())
DEFINE_BASIC_FIELD(ModelRequest, unsigned long long, sessionId, m_sessionId, table_name, "session_id", .setForeignKey())
DEFINE_BASIC_FIELD(ModelRequest, unsigned long long, serviceId, m_serviceId, table_name, "service_id", .setNotNull().setForeignKey())
DEFINE_BASIC_FIELD(ModelRequest, bool, isMonitoring, m_isMonitoring, table_name, "is_monitoring", .setNotNull())
DEFINE_BASIC_FIELD(ModelRequest, int, resultCodeId, m_resultCodeId, table_name, "result_code_id", .setForeignKey())




//DEFINE_ONE_TO_ONE(ModelRequest, ModelService, service_id, m_service, unsigned long long, serviceId, m_serviceId)
//DEFINE_ONE_TO_ONE(ModelRequest, ModelRequestType, requestType, m_requestType, unsigned long long, requestTypeId, m_requestTypeId)
//DEFINE_ONE_TO_ONE(ModelRequest, ModelSession, session, m_session, unsigned long long, sessionId, m_sessionId)

ModelRequest::field_list ModelRequest::fields = list_of<ModelRequest::field_list::value_type>
    (&ModelRequest::id)
    (&ModelRequest::timeBegin)
    (&ModelRequest::timeEnd)
    (&ModelRequest::sourceIp)
    (&ModelRequest::userName)
    (&ModelRequest::userId)
    (&ModelRequest::serviceId)
    (&ModelRequest::requestTypeId)
    (&ModelRequest::sessionId)
    (&ModelRequest::isMonitoring)
    (&ModelRequest::resultCodeId)
;

