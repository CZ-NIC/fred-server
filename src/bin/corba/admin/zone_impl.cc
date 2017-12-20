#include "src/bin/corba/Zone.hh"

#include "src/bin/corba/admin/zone_impl.hh"

#include "src/bin/corba/admin/common.hh"
#include "src/util/log/logger.hh"
#include "src/util/log/context.hh"
#include "src/libfred/zone.hh"

#include "src/bin/corba/connection_releaser.hh"

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
        ConnectionReleaser releaser;
    	try
    	{
			std::unique_ptr<LibFred::Zone::Manager> zoneMan(
					LibFred::Zone::Manager::create());

			zoneMan->addZone
					( fqdn
					, ex_period_min
					, ex_period_max
					, ttl
					, hostmaster
					, refresh
					, update_retr
					, expiry
					, minimum
					, ns_fqdn);
        }//try
        catch (...)
        {
            LOGGER(PACKAGE).error("createZone: an error has occured");
            return false;
        }//catch (...)

        return true;
    }
    bool ccReg_ZoneIf_i::updateZoneByFqdn
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
        ConnectionReleaser releaser;
    	try
    	{
			std::unique_ptr<LibFred::Zone::Manager> zoneMan(
					LibFred::Zone::Manager::create());

			zoneMan->updateZoneByFqdn
					( fqdn
					, ex_period_min
					, ex_period_max
					, ttl
					, hostmaster
					, refresh
					, update_retr
					, expiry
					, minimum
					, ns_fqdn);
        }//try
        catch (...)
        {
            LOGGER(PACKAGE).error("updateZoneByFqdn: an error has occured");
            return false;
        }//catch (...)

        return true;
    }
    bool ccReg_ZoneIf_i::updateZoneById
    (
        unsigned long long id
		, const char* fqdn
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
        ConnectionReleaser releaser;
		try
		{
			std::unique_ptr<LibFred::Zone::Manager> zoneMan(
					LibFred::Zone::Manager::create());

			zoneMan->updateZoneById
					( id
					, fqdn
					, ex_period_min
					, ex_period_max
					, ttl
					, hostmaster
					, refresh
					, update_retr
					, expiry
					, minimum
					, ns_fqdn);
		}//try
		catch (...)
		{
			LOGGER(PACKAGE).error("updateZoneById: an error has occured");
			return false;
		}//catch (...)

		return true;
    }
    bool ccReg_ZoneIf_i::createZoneNs
    (
    		const char* zone_fqdn
    		, const char* fqdn
    		, const char* addr
    )
    {
        ConnectionReleaser releaser;
		try
		{
			std::unique_ptr<LibFred::Zone::Manager> zoneMan(
					LibFred::Zone::Manager::create());

			zoneMan->addZoneNs(zone_fqdn, fqdn, addr);
		}//try
		catch (...)
		{
			LOGGER(PACKAGE).error("createZoneNs: an error has occured");
			return false;
		}//catch (...)

		return true;
    }

    bool ccReg_ZoneIf_i::updateZoneNs
    (
            unsigned long long id
    		, const char* zone_fqdn
    		, const char* fqdn
    		, const char* addr
    )
    {
        ConnectionReleaser releaser;
		try
		{
			std::unique_ptr<LibFred::Zone::Manager> zoneMan(
					LibFred::Zone::Manager::create());
			zoneMan->updateZoneNsById(id, zone_fqdn, fqdn, addr);
		}//try
		catch (...)
		{
			LOGGER(PACKAGE).error("updateZoneNs: an error has occured");
			return false;
		}//catch (...)

		return true;
    }

