#include "model_registrar_certification.h"

std::string ModelRegistrarCertification::table_name = "registrar_certification";

DEFINE_PRIMARY_KEY(ModelRegistrarCertification, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelRegistrarCertification, unsigned long long, registrar_id, m_registrar_id, table_name, "registrar_id",.setForeignKey() )
DEFINE_BASIC_FIELD(ModelRegistrarCertification, Database::Date, valid_from, m_valid_from, table_name, "valid_from", .setNotNull())
DEFINE_BASIC_FIELD(ModelRegistrarCertification, Database::Date, valid_until, m_valid_until, table_name, "valid_until", .setNotNull())
DEFINE_BASIC_FIELD(ModelRegistrarCertification, int, classification, m_classification, table_name, "classification", )
DEFINE_BASIC_FIELD(ModelRegistrarCertification, unsigned long long, eval_file_id, m_eval_file_id, table_name, "eval_file_id",.setForeignKey() )

ModelRegistrarCertification::field_list ModelRegistrarCertification::fields = list_of<ModelRegistrarCertification::field_list::value_type>
    (&ModelRegistrarCertification::id)
    (&ModelRegistrarCertification::registrar_id)
    (&ModelRegistrarCertification::valid_from)
    (&ModelRegistrarCertification::valid_until)
    (&ModelRegistrarCertification::classification)
    (&ModelRegistrarCertification::eval_file_id)
;

