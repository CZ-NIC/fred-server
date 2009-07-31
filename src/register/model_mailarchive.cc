#include "model_mailarchive.h"

std::string ModelMailArchive::table_name = "mail_archive";

DEFINE_PRIMARY_KEY(ModelMailArchive, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_FOREIGN_KEY(ModelMailArchive, ModelMailType, unsigned long long, mailTypeId, m_mailTypeId, table_name, "mailtype", id, )
DEFINE_BASIC_FIELD(ModelMailArchive, Database::DateTime, crDate, m_crDate, table_name, "crdate", .setDefault().setNotNull())
DEFINE_BASIC_FIELD(ModelMailArchive, Database::DateTime, modDate, m_modDate, table_name, "moddate", )
DEFINE_BASIC_FIELD(ModelMailArchive, int, status, m_status, table_name, "status", )
DEFINE_BASIC_FIELD(ModelMailArchive, std::string, message, m_message, table_name, "message", .setNotNull())
DEFINE_BASIC_FIELD(ModelMailArchive, short int, attempt, m_attempt, table_name, "attempt", .setDefault().setNotNull())
DEFINE_BASIC_FIELD(ModelMailArchive, std::string, response, m_response, table_name, "response", )

DEFINE_ONE_TO_ONE(ModelMailArchive, ModelMailType, mailType, m_mailType, unsigned long long, mailTypeId, m_mailTypeId)

DEFINE_ONE_TO_MANY(ModelMailArchive, attachments, unsigned long long, m_attachments, ModelMailAttachments, mailId)

ModelMailArchive::field_list ModelMailArchive::fields = list_of<ModelMailArchive::field_list::value_type>
    (&ModelMailArchive::id)
    (&ModelMailArchive::mailTypeId)
    (&ModelMailArchive::crDate)
    (&ModelMailArchive::modDate)
    (&ModelMailArchive::status)
    (&ModelMailArchive::message)
    (&ModelMailArchive::attempt)
    (&ModelMailArchive::response)
;

