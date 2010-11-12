#include "model_zone_soa.h"

std::string ModelZoneSoa::table_name = "zone_soa";

DEFINE_PRIMARY_KEY(ModelZoneSoa, unsigned long long, zone, m_zone, table_name, "zone", )
DEFINE_BASIC_FIELD(ModelZoneSoa, int, ttl, m_ttl, table_name, "ttl", .setNotNull())
DEFINE_BASIC_FIELD(ModelZoneSoa, std::string, hostmaster, m_hostmaster, table_name, "hostmaster", .setNotNull())
DEFINE_BASIC_FIELD(ModelZoneSoa, int, serial, m_serial, table_name, "serial", )
DEFINE_BASIC_FIELD(ModelZoneSoa, int, refresh, m_refresh, table_name, "refresh", .setNotNull())
DEFINE_BASIC_FIELD(ModelZoneSoa, int, updateRetr, m_updateRetr, table_name, "update_retr", .setNotNull())
DEFINE_BASIC_FIELD(ModelZoneSoa, int, expiry, m_expiry, table_name, "expiry", .setNotNull())
DEFINE_BASIC_FIELD(ModelZoneSoa, int, minimum, m_minimum, table_name, "minimum", .setNotNull())
DEFINE_BASIC_FIELD(ModelZoneSoa, std::string, nsFqdn, m_nsFqdn, table_name, "ns_fqdn", .setNotNull())

//DEFINE_ONE_TO_ONE(ModelZoneSoa, ModelZone, ftab_zone, m_ftab_zone, unsigned long long, zoneId, m_zoneId)

ModelZoneSoa::field_list ModelZoneSoa::fields = list_of<ModelZoneSoa::field_list::value_type>
    (&ModelZoneSoa::zone)
    (&ModelZoneSoa::ttl)
    (&ModelZoneSoa::hostmaster)
    (&ModelZoneSoa::serial)
    (&ModelZoneSoa::refresh)
    (&ModelZoneSoa::updateRetr)
    (&ModelZoneSoa::expiry)
    (&ModelZoneSoa::minimum)
    (&ModelZoneSoa::nsFqdn)
;

