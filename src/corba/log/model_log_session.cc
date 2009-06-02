#include "model_log_session.h"

std::string ModelLogSession::table_name = "log_session";

DEFINE_PRIMARY_KEY(ModelLogSession, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelLogSession, std::string, name, m_name, table_name, "name", .setNotNull())
DEFINE_BASIC_FIELD(ModelLogSession, Database::DateTime, loginDate, m_loginDate, table_name, "login_date", .setDefault().setNotNull())
DEFINE_BASIC_FIELD(ModelLogSession, Database::DateTime, logoutDate, m_logoutDate, table_name, "logout_date", )
DEFINE_BASIC_FIELD(ModelLogSession, std::string, loginTr, m_loginTr, table_name, "login_trid", )
DEFINE_BASIC_FIELD(ModelLogSession, std::string, logoutTr, m_logoutTr, table_name, "logout_trid", )
DEFINE_BASIC_FIELD(ModelLogSession, std::string, lang, m_lang, table_name, "lang", .setDefault().setNotNull())


ModelLogSession::field_list ModelLogSession::fields = list_of<ModelLogSession::field_list::value_type>
    (&ModelLogSession::id)
    (&ModelLogSession::name)
    (&ModelLogSession::loginDate)
    (&ModelLogSession::logoutDate)
    (&ModelLogSession::loginTr)
    (&ModelLogSession::logoutTr)
    (&ModelLogSession::lang)
;

