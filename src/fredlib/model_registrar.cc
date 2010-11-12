#include "model_registrar.h"

ModelRegistrar::ModelRegistrar() {
}


ModelRegistrar::~ModelRegistrar() {
}


std::string ModelRegistrar::table_name = "registrar";

DEFINE_PRIMARY_KEY(ModelRegistrar, unsigned long long, id, id_, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, ico, ico_, table_name, "ico",)
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, dic, dic_, table_name, "dic",)
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, varsymb, varsymb_, table_name, "varsymb",)
DEFINE_BASIC_FIELD(ModelRegistrar, bool, vat, vat_, table_name, "vat", .setDefault())
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, handle, handle_, table_name, "handle", .setNotNull())
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, name, name_, table_name, "name",)
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, organization, organization_, table_name, "organization",)
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, street1, street1_, table_name, "street1",)
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, street2, street2_, table_name, "street2",)
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, street3, street3_, table_name, "street3",)
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, city, city_, table_name, "city",)
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, stateorprovince, stateorprovince_, table_name, "stateorprovince",)
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, postalcode, postalcode_, table_name, "postalcode",)
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, country, country_, table_name, "country",)
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, telephone, telephone_, table_name, "telephone",)
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, fax, fax_, table_name, "fax",)
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, email, email_, table_name, "email",)
DEFINE_BASIC_FIELD(ModelRegistrar, std::string, url, url_, table_name, "url",)
DEFINE_BASIC_FIELD(ModelRegistrar, bool, system, system_, table_name, "system", .setDefault())

//DEFINE_ONE_TO_MANY(ModelRegistrar, cl_objects, unsigned long long, cl_objects_, ModelObject, clid)
//DEFINE_ONE_TO_MANY(ModelRegistrar, acls, unsigned long long, acls_, ModelRegistrarAcl, registrar_id)


ModelRegistrar::field_list ModelRegistrar::fields = list_of<ModelRegistrar::field_list::value_type>(&ModelRegistrar::id)
                                                                                    (&ModelRegistrar::ico)
                                                                                    (&ModelRegistrar::dic)
                                                                                    (&ModelRegistrar::varsymb)
                                                                                    (&ModelRegistrar::vat)
                                                                                    (&ModelRegistrar::handle)
                                                                                    (&ModelRegistrar::name)
                                                                                    (&ModelRegistrar::organization)
                                                                                    (&ModelRegistrar::street1)
                                                                                    (&ModelRegistrar::street2)
                                                                                    (&ModelRegistrar::street3)
                                                                                    (&ModelRegistrar::city)
                                                                                    (&ModelRegistrar::stateorprovince)
                                                                                    (&ModelRegistrar::postalcode)
                                                                                    (&ModelRegistrar::country)
                                                                                    (&ModelRegistrar::telephone)
                                                                                    (&ModelRegistrar::fax)
                                                                                    (&ModelRegistrar::email)
                                                                                    (&ModelRegistrar::url)
                                                                                    (&ModelRegistrar::system);


