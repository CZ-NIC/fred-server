#include "model_files.h"

std::string ModelFiles::table_name = "files";

DEFINE_PRIMARY_KEY(ModelFiles, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelFiles, std::string, name, m_name, table_name, "name", .setNotNull())
DEFINE_BASIC_FIELD(ModelFiles, std::string, path, m_path, table_name, "path", .setNotNull())
DEFINE_BASIC_FIELD(ModelFiles, std::string, mimeType, m_mimeType, table_name, "mimetype", .setDefault().setNotNull())
DEFINE_BASIC_FIELD(ModelFiles, Database::DateTime, crDate, m_crDate, table_name, "crdate", .setDefault().setNotNull())
DEFINE_BASIC_FIELD(ModelFiles, int, filesize, m_filesize, table_name, "filesize", .setNotNull())
DEFINE_BASIC_FIELD(ModelFiles, unsigned long long, fileTypeId, m_fileTypeId, table_name, "fileType", .setForeignKey() )

//DEFINE_ONE_TO_ONE(ModelFiles, ModelEnumFileType, fileType, m_fileType, unsigned long long, fileTypeId, m_fileTypeId)

ModelFiles::field_list ModelFiles::fields = list_of<ModelFiles::field_list::value_type>
    (&ModelFiles::id)
    (&ModelFiles::name)
    (&ModelFiles::path)
    (&ModelFiles::mimeType)
    (&ModelFiles::crDate)
    (&ModelFiles::filesize)
    (&ModelFiles::fileTypeId)
;

