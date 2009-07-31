#include "model_mailattachments.h"
#include "model_mailarchive.h"

std::string ModelMailAttachments::table_name = "mail_attachments";

DEFINE_PRIMARY_KEY(ModelMailAttachments, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_FOREIGN_KEY(ModelMailAttachments, ModelMailArchive, unsigned long long, mailId, m_mailId, table_name, "mailid", id, )
DEFINE_FOREIGN_KEY(ModelMailAttachments, ModelFiles, unsigned long long, attachId, m_attachId, table_name, "attachid", id, )

DEFINE_ONE_TO_ONE(ModelMailAttachments, ModelMailArchive, mail, m_mail, unsigned long long, mailId, m_mailId)
DEFINE_ONE_TO_ONE(ModelMailAttachments, ModelFiles, attach, m_attach, unsigned long long, attachId, m_attachId)

ModelMailAttachments::field_list ModelMailAttachments::fields = list_of<ModelMailAttachments::field_list::value_type>
    (&ModelMailAttachments::id)
    (&ModelMailAttachments::mailId)
    (&ModelMailAttachments::attachId)
;

