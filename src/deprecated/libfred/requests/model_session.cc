#include "src/deprecated/libfred/requests/model_session.hh"

std::string ModelSession::table_name = "session";

DEFINE_PRIMARY_KEY(ModelSession, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelSession, std::string, userName, m_userName, table_name, "user_name", .setNotNull())
DEFINE_BASIC_FIELD(ModelSession, unsigned long long, userId, m_userId, table_name, "user_id", )
DEFINE_BASIC_FIELD(ModelSession, Database::DateTime, loginDate, m_loginDate, table_name, "login_date", .setDefault().setNotNull())
DEFINE_BASIC_FIELD(ModelSession, Database::DateTime, logoutDate, m_logoutDate, table_name, "logout_date", )


ModelSession::field_list ModelSession::fields = list_of<ModelSession::field_list::value_type>
    (&ModelSession::id)
    (&ModelSession::userName)
    (&ModelSession::userId)
    (&ModelSession::loginDate)
    (&ModelSession::logoutDate)
;

