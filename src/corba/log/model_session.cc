#include "model_session.h"

std::string ModelSession::table_name = "session";

DEFINE_PRIMARY_KEY(ModelSession, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelSession, std::string, name, m_name, table_name, "name", .setNotNull())
DEFINE_BASIC_FIELD(ModelSession, Database::DateTime, loginDate, m_loginDate, table_name, "login_date", .setDefault().setNotNull())
DEFINE_BASIC_FIELD(ModelSession, Database::DateTime, logoutDate, m_logoutDate, table_name, "logout_date", )
DEFINE_BASIC_FIELD(ModelSession, std::string, lang, m_lang, table_name, "lang", .setDefault().setNotNull())


ModelSession::field_list ModelSession::fields = list_of<ModelSession::field_list::value_type>
    (&ModelSession::id)
    (&ModelSession::name)
    (&ModelSession::loginDate)
    (&ModelSession::logoutDate)
    (&ModelSession::lang)
;

