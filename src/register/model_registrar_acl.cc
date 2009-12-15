#include "model_registrar_acl.h"


std::string ModelRegistrarAcl::table_name = "registraracl";

DEFINE_PRIMARY_KEY(ModelRegistrarAcl, unsigned long long, id, id_, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelRegistrarAcl, unsigned long long, registrar_id, registrar_id_, table_name, "registrarid", .setNotNull().setForeignKey())
DEFINE_BASIC_FIELD(ModelRegistrarAcl, std::string, cert, cert_, table_name, "cert", .setNotNull())
DEFINE_BASIC_FIELD(ModelRegistrarAcl, std::string, password, password_, table_name, "password", .setNotNull())

ModelRegistrarAcl::field_list ModelRegistrarAcl::fields = list_of<ModelRegistrarAcl::field_list::value_type>(&ModelRegistrarAcl::id)
                                                                                             (&ModelRegistrarAcl::registrar_id)
                                                                                             (&ModelRegistrarAcl::cert)
                                                                                             (&ModelRegistrarAcl::password);

