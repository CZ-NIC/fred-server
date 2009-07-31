#include "model_object_registry.h"


std::string ModelObjectRegistry::table_name = "object_registry";

DEFINE_PRIMARY_KEY(ModelObjectRegistry, unsigned long long, id, id_, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelObjectRegistry, std::string, roid, roid_, table_name, "roid", .setNotNull())
DEFINE_BASIC_FIELD(ModelObjectRegistry, int, type, type_, table_name, "type",)
DEFINE_BASIC_FIELD(ModelObjectRegistry, std::string, name, name_, table_name, "name", .setNotNull())
DEFINE_FOREIGN_KEY(ModelObjectRegistry, ModelRegistrar, unsigned long long, crid, crid_, table_name, "crid", id, .setNotNull())
DEFINE_BASIC_FIELD(ModelObjectRegistry, ptime, crdate, crdate_, table_name, "crdate", .setNotNull().setDefault())
DEFINE_BASIC_FIELD(ModelObjectRegistry, ptime, erdate, erdate_, table_name, "erdate",)
DEFINE_ONE_TO_ONE (ModelObjectRegistry, ModelRegistrar, cr, cr_, unsigned long long, crid, crid_)


ModelObjectRegistry::field_list ModelObjectRegistry::fields = list_of<ModelObjectRegistry::field_list::value_type>(&ModelObjectRegistry::id)
                                                                                                   (&ModelObjectRegistry::roid)
                                                                                                   (&ModelObjectRegistry::type)
                                                                                                   (&ModelObjectRegistry::name)
                                                                                                   (&ModelObjectRegistry::crid)
                                                                                                   (&ModelObjectRegistry::crdate)
                                                                                                   (&ModelObjectRegistry::erdate);


