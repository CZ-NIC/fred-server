#include <corba/ccReg.hh>

#include "zone_impl.h"

#include "common.h"
#include "log/logger.h"
#include "log/context.h"
#include "register/zone.h"

    bool ccReg_ZoneIf_i::createZone
    (
    		const char* fqdn
    		, CORBA::Long ex_period_min
    		, CORBA::Long ex_period_max
    		, CORBA::Long ttl
    		, const char* hostmaster
    		, CORBA::Long refresh
    		, CORBA::Long update_retr
    		, CORBA::Long expiry
    		, CORBA::Long minimum
    		, const char* ns_fqdn
    )
    {
        std::auto_ptr<Register::Zone::Manager> zoneMan(
                Register::Zone::Manager::create());

    	return false;
    }
    bool ccReg_ZoneIf_i::updateZone
    (
    		const char* fqdn
    		, CORBA::Long ex_period_min
    		, CORBA::Long ex_period_max
    		, CORBA::Long ttl
    		, const char* hostmaster
    		, CORBA::Long refresh
    		, CORBA::Long update_retr
    		, CORBA::Long expiry
    		, CORBA::Long minimum
    		, const char* ns_fqdn
    )
    {
    	return false;
    }
    bool ccReg_ZoneIf_i::createZoneNs
    (
    		const char* zone_fqdn
    		, const char* fqdn
    		, const char* addr
    )
    {
    	return false;
    }

    bool ccReg_ZoneIf_i::updateZoneNs
    (
    		ccReg::TID id
    		, const char* zone_fqdn
    		, const char* fqdn
    		, const char* addr
    )
    {
    	return false;
    }

    bool ccReg_ZoneIf_i::createZoneSoa
    (
    		const char* zone_fqdn
    		, CORBA::Long ttl
    		, const char* hostmaster
    		, CORBA::Long serial
    		, CORBA::Long refresh
    		, CORBA::Long update_retr
    		, CORBA::Long expiry
    		, CORBA::Long minimum
    		, const char* ns_fqdn
    )
    {
    	return false;
    }

    bool ccReg_ZoneIf_i::updateZoneSoa
    (
    		const char* zone_fqdn
    		, CORBA::Long ttl
    		, const char* hostmaster
    		, CORBA::Long serial
    		, CORBA::Long refresh
    		, CORBA::Long update_retr
    		, CORBA::Long expiry
    		, CORBA::Long minimum
    		, const char* ns_fqdn
    )
    {
    	return false;
    }
