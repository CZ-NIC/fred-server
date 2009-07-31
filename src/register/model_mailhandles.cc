#include "model_mailhandles.h"

std::string ModelMailHandles::table_name = "mail_handles";

DEFINE_PRIMARY_KEY(ModelMailHandles, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_FOREIGN_KEY(ModelMailHandles, ModelMailArchive, unsigned long long, mailId, m_mailId, table_name, "mailid", id, )
DEFINE_BASIC_FIELD(ModelMailHandles, std::string, assoc, m_assoc, table_name, "associd", )

DEFINE_ONE_TO_ONE(ModelMailHandles, ModelMailArchive, mail, m_mail, unsigned long long, mailId, m_mailId)

ModelMailHandles::field_list ModelMailHandles::fields = list_of<ModelMailHandles::field_list::value_type>
    (&ModelMailHandles::id)
    (&ModelMailHandles::mailId)
    (&ModelMailHandles::assoc)
;

