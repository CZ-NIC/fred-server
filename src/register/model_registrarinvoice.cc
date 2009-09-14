#include "model_registrarinvoice.h"

std::string ModelRegistrarinvoice::table_name = "registrarinvoice";

DEFINE_PRIMARY_KEY(ModelRegistrarinvoice, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_FOREIGN_KEY(ModelRegistrarinvoice, ModelRegistrar, unsigned long long, registrarId, m_registrarId, table_name, "registrarid", id, .setNotNull())
DEFINE_FOREIGN_KEY(ModelRegistrarinvoice, ModelZone, unsigned long long, zoneId, m_zoneId, table_name, "zone", id, .setNotNull())
DEFINE_BASIC_FIELD(ModelRegistrarinvoice, Database::Date, fromDate, m_fromDate, table_name, "fromdate", .setNotNull())
DEFINE_BASIC_FIELD(ModelRegistrarinvoice, Database::Date, lastDate, m_lastDate, table_name, "lastdate", )
DEFINE_BASIC_FIELD(ModelRegistrarinvoice, Database::Date, toDate, m_toDate, table_name, "todate", )

DEFINE_ONE_TO_ONE(ModelRegistrarinvoice, ModelRegistrar, ftab_registrar, m_ftab_registrar, unsigned long long, registrarId, m_registrarId)
DEFINE_ONE_TO_ONE(ModelRegistrarinvoice, ModelZone, ftab_zone, m_ftab_zone, unsigned long long, zoneId, m_zoneId)

ModelRegistrarinvoice::field_list ModelRegistrarinvoice::fields = list_of<ModelRegistrarinvoice::field_list::value_type>
    (&ModelRegistrarinvoice::id)
    (&ModelRegistrarinvoice::registrarId)
    (&ModelRegistrarinvoice::zoneId)
    (&ModelRegistrarinvoice::fromDate)
    (&ModelRegistrarinvoice::lastDate)
    (&ModelRegistrarinvoice::toDate)
;

