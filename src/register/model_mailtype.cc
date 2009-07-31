#include "model_mailtype.h"

std::string ModelMailType::table_name = "mail_type";

DEFINE_PRIMARY_KEY(ModelMailType, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelMailType, std::string, name, m_name, table_name, "name", .setNotNull())
DEFINE_BASIC_FIELD(ModelMailType, std::string, subject, m_subject, table_name, "subject", .setNotNull())


ModelMailType::field_list ModelMailType::fields = list_of<ModelMailType::field_list::value_type>
    (&ModelMailType::id)
    (&ModelMailType::name)
    (&ModelMailType::subject)
;

