#include "model_zone.h"

std::string ModelZone::table_name = "zone";

DEFINE_PRIMARY_KEY(ModelZone, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelZone, std::string, fqdn, m_fqdn, table_name, "fqdn", .setNotNull())
DEFINE_BASIC_FIELD(ModelZone, int, exPeriodMin, m_exPeriodMin, table_name, "ex_period_min", .setNotNull())
DEFINE_BASIC_FIELD(ModelZone, int, exPeriodMax, m_exPeriodMax, table_name, "ex_period_max", .setNotNull())
DEFINE_BASIC_FIELD(ModelZone, int, valPeriod, m_valPeriod, table_name, "val_period", .setNotNull())
DEFINE_BASIC_FIELD(ModelZone, int, dotsMax, m_dotsMax, table_name, "dots_max", .setNotNull().setDefault())
DEFINE_BASIC_FIELD(ModelZone, bool, enumZone, m_enumZone, table_name, "enum_zone", .setDefault())

ModelZone::field_list ModelZone::fields = list_of<ModelZone::field_list::value_type>
    (&ModelZone::id)
    (&ModelZone::fqdn)
    (&ModelZone::exPeriodMax)
    (&ModelZone::exPeriodMin)
    (&ModelZone::valPeriod)
    (&ModelZone::dotsMax)
    (&ModelZone::enumZone);
