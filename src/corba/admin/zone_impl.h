#ifndef ZONE_IMPL_H_
#define ZONE_IMPL_H_

#include <string>
#include "register/zone.h"

class ccReg_ZoneIf_i:
    public POA_ccReg::ZoneIf,
    public PortableServer::RefCountServantBase
{
private:
    std::string m_connection_string;

public:
    ccReg_ZoneIf_i()
    { }
    ~ccReg_ZoneIf_i()
    { }

    bool createZone
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
    );
    bool updateZone
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
    );
    bool createZoneNs
    (
    		const char* zone_fqdn
    		, const char* fqdn
    		, const char* addr
    );
    bool updateZoneNs
    (
    		ccReg::TID id
    		, const char* zone_fqdn
    		, const char* fqdn
    		, const char* addr
    );
    bool createZoneSoa
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
    );
    bool updateZoneSoa
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
    );

};//class ccReg_ZoneIf_i

#endif //ZONE_IMPL_H_
