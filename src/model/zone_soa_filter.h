#ifndef _ZONE_SOA_FILTER_H_
#define _ZONE_SOA_FILTER_H_

#include "db/query/base_filters.h"
#include "zone_filter.h"

namespace Database {
namespace Filters {

class ZoneSoa:
    virtual public Zone {
public:
    virtual ~ZoneSoa()
    { }

    virtual Table &joinZoneSoaTable() = 0;
    virtual Value<Database::ID> &addZoneId() = 0;
    virtual Value<int> &addTtl() = 0;
    virtual Value<std::string> &addHostmaster() = 0;
    virtual Value<int> &addSerial() = 0;
    virtual Value<int> &addRefresh() = 0;
    virtual Value<int> &addUpdateRetr() = 0;
    virtual Value<int> &addExpiry() = 0;
    virtual Value<int> &addMinimum() = 0;
    virtual Value<std::string> &addNsFqdn() = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
            const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }
}; // class ZoneSoa

class ZoneSoaImpl
	: public ZoneSoa
    , virtual public ZoneImpl
{
public:
    ZoneSoaImpl(bool set_active = false);
    virtual ~ZoneSoaImpl();

    virtual Table &joinZoneSoaTable();
    virtual void _joinPolymorphicTables();
    virtual Value<Database::ID> &addZoneId();
    virtual Value<int> &addTtl();
    virtual Value<std::string> &addHostmaster();
    virtual Value<int> &addSerial();
    virtual Value<int> &addRefresh();
    virtual Value<int> &addUpdateRetr();
    virtual Value<int> &addExpiry();
    virtual Value<int> &addMinimum();
    virtual Value<std::string> &addNsFqdn();

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
            const unsigned int _version) 
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ZoneSoa);
    }
}; // class ZoneSoaImpl

} // namespace Filters
} // namespace Database

#endif // _ZONE_SOA_FILTER_H_
