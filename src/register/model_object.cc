#include "model_object.h"

std::string ModelObject::table_name = "object";

DEFINE_PRIMARY_KEY(ModelObject, unsigned long long, id, id_, table_name, "id",)
DEFINE_FOREIGN_KEY(ModelObject, ModelRegistrar, unsigned long long, clid, clid_, table_name, "clid", id, .setNotNull())
DEFINE_FOREIGN_KEY(ModelObject, ModelRegistrar, unsigned long long, upid, upid_, table_name, "upid", id,)
DEFINE_BASIC_FIELD(ModelObject, ptime, tr_date, tr_date_, table_name, "trdate",)
DEFINE_BASIC_FIELD(ModelObject, ptime, up_date, up_date_, table_name, "update",)
DEFINE_BASIC_FIELD(ModelObject, std::string, authinfopw, authinfopw_, table_name, "authinfopw",)

ModelObject::field_list ModelObject::fields = list_of<ModelObject::field_list::value_type>(&ModelObject::id)
                                                                           (&ModelObject::clid)
                                                                           (&ModelObject::upid)
                                                                           (&ModelObject::tr_date)
                                                                           (&ModelObject::up_date)
                                                                           (&ModelObject::authinfopw);



std::string ModelObjectHistory::table_name = "object_history";

DEFINE_PRIMARY_KEY(ModelObjectHistory, unsigned long long, history_id, history_id_, table_name, "historyid", .setDefault())
DEFINE_FOREIGN_KEY(ModelObjectHistory, ModelObjectRegistry, unsigned long long, id, id_, table_name, "id", id, .setNotNull())
DEFINE_FOREIGN_KEY(ModelObjectHistory, ModelRegistrar, unsigned long long, clid, clid_, table_name, "clid", id, .setNotNull())
DEFINE_FOREIGN_KEY(ModelObjectHistory, ModelRegistrar, unsigned long long, upid, upid_, table_name, "upid", id,)
DEFINE_BASIC_FIELD(ModelObjectHistory, ptime, tr_date, tr_date_, table_name, "trdate",)
DEFINE_BASIC_FIELD(ModelObjectHistory, ptime, up_date, up_date_, table_name, "update",)
DEFINE_BASIC_FIELD(ModelObjectHistory, std::string, authinfopw, authinfopw_, table_name, "authinfopw",)

ModelObjectHistory::field_list ModelObjectHistory::fields = list_of<ModelObjectHistory::field_list::value_type>(&ModelObjectHistory::history_id)
                                                                                                (&ModelObjectHistory::id)
                                                                                                (&ModelObjectHistory::clid)
                                                                                                (&ModelObjectHistory::upid)
                                                                                                (&ModelObjectHistory::tr_date)
                                                                                                (&ModelObjectHistory::up_date)
                                                                                                (&ModelObjectHistory::authinfopw);


