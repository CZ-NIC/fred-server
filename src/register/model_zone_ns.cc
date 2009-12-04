#include "model_zone_ns.h"

std::string ModelZoneNs::table_name = "zone_ns";

DEFINE_PRIMARY_KEY(ModelZoneNs, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelZoneNs, unsigned long long, zoneId, m_zoneId, table_name, "zone", .setForeignKey() )
DEFINE_BASIC_FIELD(ModelZoneNs, std::string, fqdn, m_fqdn, table_name, "fqdn", .setNotNull())
DEFINE_BASIC_FIELD(ModelZoneNs, std::string, addrs, m_addrs, table_name, "addrs", .setNotNull())

//DEFINE_ONE_TO_ONE(ModelZoneNs, ModelZone, zone, m_zone, unsigned long long, zoneId, m_zoneId)

ModelZoneNs::field_list ModelZoneNs::fields = list_of<ModelZoneNs::field_list::value_type>
    (&ModelZoneNs::id)
    (&ModelZoneNs::zoneId)
    (&ModelZoneNs::fqdn)
    (&ModelZoneNs::addrs)
;

